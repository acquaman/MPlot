#ifndef MPLOTOBSERVER_H
#define MPLOTOBSERVER_H


class MPlotObservable;

class MPlotObserver {
public:
	MPlotObserver() {}

	virtual void onObservableChanged(MPlotObservable* source, int code, const char* msg, int payload) = 0;

};


#endif // MPLOTOBSERVER_H
