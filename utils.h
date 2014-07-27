#pragma once

#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <gdkmm.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/lexical_cast.hpp>
#include <fann.h>
#include <gtkmm.h>

#include "Point.h"

namespace Utils {
	typedef boost::shared_ptr<const IplImage> SharedImage;

	#define xForEach(iter, container) \
		for (typeof(container.begin()) iter = container.begin(); iter != container.end(); iter++)

	#define xForEachBack(iter, container) \
		for (typeof(container.rbegin()) iter = container.rbegin(); iter != container.rend(); iter++)

	struct QuitNow: public std::exception {
		using std::exception::what;

		QuitNow() { }
		virtual ~QuitNow() throw() { }
		virtual const char* what() throw() {
			return "QuitNow: request normal termination of program.\n(You should not see this message. Please report it if you do.)";
		}
	};

	template <class T> inline T square(T a) {
		return a * a;
	}

	template <class T> ostream &operator<< (ostream &out, vector<T> const &vec) {
		out << vec.size() << endl;
		xForEach(iter, vec)
		out << *iter << endl;
		return out;
	}

	template <class T> istream &operator>> (istream &in, vector<T> &vec) {
		int size;
		T element;

		vec.clear();
		in >> size;
		for (int i = 0; i < size; i++) {
			in >> element;
			vec.push_back(element);
		}

		return in;
	}

	template <class T> T teeFunction(T source, char *prefix, char *postfix="\n") {
		cout << prefix << source << postfix;
		return source;
	}

	#define debugTee(x) teeFunction(x, #x ": ")

	template <class T> void saveVector(CvFileStorage *out, const char *name, vector<T> &vec) {
		cvStartWriteStruct(out, name, CV_NODE_SEQ);
		for (int i = 0; i < vec.size(); i++) {
			vec[i].save(out);
		}
		cvEndWriteStruct(out);
	}

	template <class T> vector<T> loadVector(CvFileStorage *in, CvFileNode *node) {
		CvSeq *seq = node->data.seq;
		CvSeqReader reader;

		cvStartReadSeq(seq, &reader, 0);
		vector<T> result(seq->total);

		for (int i = 0; i < seq->total; i++) {
			CvFileNode *item = (CvFileNode *)reader.ptr;
			result[i].load(in, item);
			CV_NEXT_SEQ_ELEM(seq->elem_size, reader);
		}

		return result;
	}

	template <class From, class To> void convert(const From &from, To &to) {
		to = from;
	}

	template <class From, class To> void convert(const vector<From> &from, vector<To> &to) {
		to.resize(from.size());
		for (int i = 0; i < (int)from.size(); i++) {
			convert(from[i], to[i]);
		}
	}

	class ConstancyDetector {
	public:
		ConstancyDetector(int maxCounter):
			_value(-1),
			_counter(0),
			_maxCounter(maxCounter)
		{
		}

		bool isStable() {
			return _counter >= _maxCounter;
		}

		bool isStableExactly() {
			return _counter == _maxCounter;
		}

		bool observe(int newValue) {
			if (newValue != _value || newValue < 0) {
				_counter = 0;
			} else {
				_counter++;
			}

			_value = newValue;
			return isStable();
		}

		void reset() {
			_counter = 0;
			_value = -1;
		}

	private:
		int _value;
		int _counter;
		int _maxCounter;
	};

	// #define output(X) { cout << #X " = " << X << endl; }

	template <class T> int maxAbsIndex(T const &vec, int size) {
		int maxIndex = 0;

		for (int i = 0; i < size; i++) {
			if (fabs(vec[i]) > fabs(vec[maxIndex])) {
				maxIndex = i;
			}
		}

		return maxIndex;
	}


	boost::shared_ptr<IplImage> createImage(const CvSize &size, int depth, int channels);
	void releaseImage(IplImage *image);

	void mapToFirstMonitorCoordinates(Point monitor2Point, Point &monitor1Point);
	void mapToVideoCoordinates(Point monitor2Point, double resolution, Point &videoPoint, bool reverseX=true);
	void mapToNeuralNetworkCoordinates(Point point, Point &nnPoint);
	void mapFromNeuralNetworkToScreenCoordinates(Point nnPoint, Point &point);

	string getUniqueFileName(string directory, string baseFileName);

	void normalizeGrayScaleImage(IplImage *image, double standardMean=127, double standardStd=50);
	void normalizeGrayScaleImage2(IplImage *image, double standardMean=127, double standardStd=50);

	void printMat(CvMat *mat);
}

namespace boost {
	template <> void checked_delete(IplImage *image);
}
