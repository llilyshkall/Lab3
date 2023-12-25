using System;

namespace Lab3
{
    class Program
    {
        static void Main(string[] args)
        {
            VDataArray dataList = new VDataArray(9, 0, 4);
            for (int i = 0; i < dataList.Length; ++i)
            {
                double x = dataList.DataX[i];
                dataList.DataY[i] = 3 * x * x * x * x - 16 * x * x * x + 18 * x * x + 2 * x + 1;
            }
            SplineData spline = new SplineData(dataList, 5, 20);
            spline.func();
            Console.WriteLine(spline.ToLongString("{0:f4}"));
        }
    }
}
