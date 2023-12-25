using System;
using System.IO;
using System.Collections;
namespace Lab3
{
	public class VDataArray
	{
		public double[] DataX { get; set; }
		public double[] DataY { get; set; }
		public int Length { get; }
		public VDataArray(int nX, double xL, double xR)
		{
			Length = nX;
			DataX = new double[nX];
			DataY = new double[nX];
			double step = 0;
			if (nX != 0)
				step = (xR - xL) / (nX - 1);
			for (int i = 0; i < nX; ++i)
			{
				DataX[i] = xL + step * i;
				DataY[i] = 0;
			}
		}
		public override string ToString() =>
			$"V1DataArray: countItems: {DataX.Length}\n";
		public string ToLongString(string format)
		{
			string ret = ToString();
			for (int i = 0; i < DataX.Length; ++i)
			{
				ret = ret + $"   " +
					$" x: {string.Format(format, DataX[i])}," +
					$" y: {string.Format(format, DataY[i])}\n";
			}
			return ret;
		}
	}
}

