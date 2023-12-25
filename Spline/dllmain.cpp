#include "pch.h"
#include "mkl.h"
#include <iostream>

enum class ErrorEnum { NO, INIT, CHECK, SOLVE, JACOBI, GET, DELET, RCI };

typedef void (*FValues) (MKL_INT* m, MKL_INT* n, double* x, double* f, void* user_data);

class SupportData {
public:
	const double* X;
	const double* Y;
	int argMin, argMax;
	SupportData(const int size, const double* x, const double* y)
		: X(x), Y(y) {
		argMin = 0;
		argMax = 0;
		for (int i = 1; i < size; ++i) {
			if (x[i] < x[argMin])
				argMin = i;
			if (x[i] > x[argMax])
				argMax = i;
		}
	}
};

using namespace std;

void TestFunction(MKL_INT* m, MKL_INT* n, double* y, double* f, void* user_data) {
	MKL_INT s_order = DF_PP_CUBIC; // степень кубического сплайна
	MKL_INT s_type = DF_PP_NATURAL; // тип сплайна
	MKL_INT bc_type = DF_BC_2ND_LEFT_DER | DF_BC_2ND_RIGHT_DER; // тип граничных условий

	double* scoeff = new double[(*n - 1) * s_order];
	SupportData* data = (SupportData*)user_data;

	double x[] = { data->X[data->argMin], data->X[data->argMax] };
	try
	{
		DFTaskPtr task;
		int status = -1;

		// Cоздание задачи (task) 
		status = dfdNewTask1D(&task,
			*n, x, DF_UNIFORM_PARTITION,
			1, y, DF_NO_HINT); 
		if (status != DF_STATUS_OK) throw 1;

		// Настройка параметров задачи 
		double bc[2]{ 0.0, 0.0 }; // массив граничных значений
		status = dfdEditPPSpline1D(task,
			s_order, s_type, bc_type, bc,
			DF_NO_IC, NULL, 
			scoeff, DF_NO_HINT);  
		if (status != DF_STATUS_OK) throw 2;

		// Создание сплайна 
		status = dfdConstruct1D(task, DF_PP_SPLINE, DF_METHOD_STD); 
		if (status != DF_STATUS_OK) throw 3;

		// Вычисление значений сплайна и его производныx
		int nDorder = 1;
		MKL_INT dorder[] = { 1 };
		status = dfdInterpolate1D(task,
			DF_INTERP, DF_METHOD_PP, 
			*m, data->X,
			DF_NON_UNIFORM_PARTITION, 
			nDorder, dorder, NULL,
			f, DF_NO_HINT, NULL); 
		if (status != DF_STATUS_OK) throw 4;

		// Освобождение ресурсов 
		status = dfDeleteTask(&task);
		if (status != DF_STATUS_OK) throw 6;
	}
	catch (int ret)
	{
		delete[] scoeff;
		return;
	}

	for (int i = 0; i < *m; ++i) f[i] -= data->Y[i];
	delete[] scoeff;
}

bool TrustRegion(
	MKL_INT n, // число независимых переменных
	MKL_INT m, // число компонент векторной функции
	double* x, // начальное приближение и решение
	FValues FValues, // указатель на функцию, вычисляющую векторную
	// функцию в заданной точке
	const double* eps, // массив с 6 элементами, определяющих критерии 
	// остановки итерационного процесса
	double jac_eps, // точность вычисления элементов матрицы Якоби 
	MKL_INT niter1, // максимальное число итераций
	MKL_INT niter2, // максимальное число итераций при выборе пробного шага
	double rs, // начальный размер доверительной области
	MKL_INT& ndoneIter, // число выполненных итераций
	double& resInitial, // начальное значение невязки
	double& resFinal, // финальное значение невязки 
	MKL_INT& stopCriteria,// выполненный критерий остановки 
	MKL_INT* checkInfo, // информация об ошибках при проверке данных 
	SupportData* data) // информация об ошибках 
{
	_TRNSP_HANDLE_t handle = NULL; // переменная для дескриптора задачи
	double* fvec = NULL; // массив значений векторной функции
	double* fjac = NULL; // массив с элементами матрицы Якоби
	fvec = new double[m]; // массив значений векторной функции
	fjac = new double[n * m]; // массив с элементами матрицы Якоби

	// Инициализация задачи
	MKL_INT ret = dtrnlsp_init(&handle, &n, &m, x, eps, &niter1, &niter2, &rs);

	// Проверка корректности входных данных 
	ret = dtrnlsp_check(&handle, &n, &m, fjac, fvec, eps, checkInfo);
	MKL_INT RCI_Request = 0;

	// Итерационный процесс
	while (true)
	{
		ret = dtrnlsp_solve(&handle, fvec, fjac, &RCI_Request);
		if (ret != TR_SUCCESS) break;
		if (RCI_Request == 0) continue;
		else if (RCI_Request == 1) TestFunction(&m, &n, x, fvec, data);
		else if (RCI_Request == 2)
		{
			ret = djacobix(TestFunction, &n, &m, fjac, x, &jac_eps, data);
		}
		else break;
	}

	// Завершение итерационного процесса
	ret = dtrnlsp_get(&handle, &ndoneIter, &stopCriteria, &resInitial, &resFinal);

	// Освобождение ресурсов
	ret = dtrnlsp_delete(&handle);

	// Освобождение памяти
	if (fvec != NULL) delete[] fvec;
	if (fjac != NULL) delete[] fjac;
	return 0;
}


extern "C" _declspec(dllexport)
int CubicSpline(
	int nX, int m, int maxIter,
	double* X, 
	double* Y,
	double* YSpline, 
	double& minRes,
	int& countIter,
	int& status)
{
	setlocale(LC_ALL, ""); // использование кириллицы

	SupportData data(nX, X, Y);

	double* y_ret = new double[m]; // массив с начальным приближением и решением 
	for (int i = 0; i < m; ++i)
		y_ret[i] = 0;
	y_ret[0] = data.Y[data.argMin];
	y_ret[m - 1] = data.Y[data.argMax];

	MKL_INT niter1 = maxIter; // максимальное число итераций
	MKL_INT niter2 = maxIter / 2; // максимальное число итераций при выборе пробного шага
	double rs = 10; // начальное значение для доверительного интервала

	// массив критериев остановки
	const double eps[6] = { 1.0E-12 , 1.0E-12 , 1.0E-12 , 1.0E-12 , 1.0E-12 , 1.0E-12 }; 
	double jac_eps = 1.0E-8; // точность вычисления элементов матрицы Якоби
	double res_initial = 0; // начальное значение невязки 

	MKL_INT check_data_info[4]; // результат проверки корректности данных 
	TrustRegion(m, nX, y_ret, TestFunction, eps, jac_eps, niter1, niter2, rs,
		countIter, res_initial, minRes, status,
		check_data_info, &data);

	TestFunction(&nX, &m, y_ret, YSpline, &data);
	for (int i = 0; i < nX; ++i)
		YSpline[i] += Y[i];

	return 0;
}
