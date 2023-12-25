using System.IO;
using System.Collections.Generic;
using System;
using System.Runtime.InteropServices;

namespace Lab3
{
    class SplineData
    {
        public VDataArray Data { get; }
        public int M { get; }
        public double[] YSpline { get; }
        public int MaxIter { get; }
        public int Status { get; set; }
        public double MinRes { get; set; }
        public List<SplineDataItem> Res { get; }
        public int CountIter { get; set; }

        public SplineData(VDataArray data, int m, int maxIter)
        {
            Data = data;
            M = m;
            YSpline = new double[data.Length];
            Status = 0;
            MaxIter = maxIter;
            MinRes = double.NaN;
            CountIter = 0;
        }
        public string ToLongString(string format)
        {
            string ret = Data.ToLongString(format);
            ret += "\nSpline\n";
            for (int i = 0; i < Data.Length; ++i)
            {
                ret += $"{string.Format("{0:d4}", i)}   " +
                       $"{string.Format(format, Data.DataX[i])}     " +
                       $"{string.Format(format, Data.DataY[i])}     " +
                       $"{string.Format(format, YSpline[i])} \n";
            }
            ret += $"\nМинимальное значение невязки: {string.Format(format, MinRes)}\n";
            ret += $"Статус";
            if (Status == 1)
                ret += " превышено заданное число итераций ";
            else if (Status == 2)
                ret += " размер доверительной области < 1e-12";
            else if (Status == 3)
                ret += " норма невязки < 1e-12";
            else if (Status == 4)
                ret += " норма строк матрицы Якоби < 1e-12";
            else if (Status == 5)
                ret += " пробный шаг < 1e-12";
            else if (Status == 6)
                ret += " разность нормы функции и погрешности < 1e-12";
            ret += $"\nКоличество итераций {CountIter}\n";
            return ret;
        }
        public bool Save(string filename, string format)
        {
            try
            {
                string str = ToLongString(format);
                using (StreamWriter writer = new StreamWriter(filename))
                {
                    writer.WriteLine(str);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Func Save. Exception: {ex.Message}\n");
                return false;
            }
            return true;
        }
        public void func()
        {
            try
            {
                int countIter = 0;
                int status = 0;
                double minRes = MinRes;
                int ret = CubicSpline(Data.Length,
                                      M,
                                      MaxIter,
                                      Data.DataX,
                                      Data.DataY,
                                      YSpline,
                                      ref minRes,
                                      ref countIter,
                                      ref status);
                CountIter = countIter;
                Status = status;
                MinRes = minRes;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Func Save. Exception: {ex.Message}\n");
            }
        }

        [DllImport("C:\\Users\\79251\\Desktop\\Suprun\\Spline\\x64\\Debug\\Spline.dll",
        CallingConvention = CallingConvention.Cdecl)]
        public static extern
        int CubicSpline(
            int nX, int m, int maxIter,
            double[] X, double[] Y,
            double[] YSpline, ref double minRes, ref int countIter, 
            ref int status);
    }
}
