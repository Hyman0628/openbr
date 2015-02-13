#include <openbr/plugins/openbr_internal.h>

using namespace cv;

namespace br
{

/*!
 * \ingroup transforms
 * \brief Quantizes the values into bins.
 * \author Josh Klontz \cite jklontz
 */
class HistBinTransform : public UntrainableTransform
{
    Q_OBJECT
    Q_PROPERTY(float min READ get_min WRITE set_min RESET reset_min STORED false)
    Q_PROPERTY(float max READ get_max WRITE set_max RESET reset_max STORED false)
    Q_PROPERTY(int bins READ get_bins WRITE set_bins RESET reset_bins STORED false)
    Q_PROPERTY(bool split READ get_split WRITE set_split RESET reset_split STORED false)
    BR_PROPERTY(float, min, 0)
    BR_PROPERTY(float, max, 255)
    BR_PROPERTY(int, bins, 8)
    BR_PROPERTY(bool, split, false)

    void project(const Template &src, Template &dst) const
    {
        const double floor = ((src.m().depth() == CV_32F) || (src.m().depth() == CV_64F)) ? -0.5 : 0;

        Mat weights, vals;
        const int channels = src.m().channels();
        if (channels == 1) {
            vals = src;
        } else if (channels == 2) {
            // If there are two channels, the first is channel is assumed to be a weight vector
            // and the second channel contains the vectors we would like to bin.
            std::vector<Mat> mv;
            cv::split(src, mv);
            weights = mv[0];
            weights.convertTo(weights, CV_32F);
            vals = mv[1];
        } else {
            qFatal("Invalid channel count: %d", channels);
        }

        vals.convertTo(dst, bins > 256 ? CV_16U : CV_8U, bins/(max-min), floor);
        if (!split) return;

        QList<Mat> outputs; outputs.reserve(bins);
        for (int i=0; i<bins; i++) {
            Mat output = (dst == i);
            if (weights.data) {
                output.convertTo(output, CV_32F);
                multiply(output, weights, output);
                output.convertTo(output, CV_8U);
            }
            outputs.append(output);
        }
        dst.clear(); dst.append(outputs);
    }
};

BR_REGISTER(Transform, HistBinTransform)

} // namespace br

#include "histbin.moc"
