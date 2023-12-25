using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Lab3
{
    struct SplineDataItem
    {
        public double X { get; set; }
        public double Y { get; set; }
        public double YSpline { get; set; }
        public SplineDataItem(double x, double y, double ySpline = double.NaN)
        {
            X = x;
            Y = y;
            YSpline = ySpline;
        }
        public string ToLongString(string format) =>
            $"Item: x: {string.Format(format, X)}, " +
            $"y: {string.Format(format, Y)}, " +
            $"y_spline:  {string.Format(format, YSpline)}\n";

        public override string ToString() =>
            $"x: {X}, y: {Y}, y_spline: {YSpline}\n";
    }
}
