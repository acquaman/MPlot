#ifndef MPLOTOBSERVABLE_CPP
#define MPLOTOBSERVABLE_CPP

#include "MPlotObservable.h"


MPlotObservable::MPlotObservable()
{
}


MPlotObservable::~MPlotObservable()
{
}

/// connect an observer (ie: subscribe)
void MPlotObservable::addObserver ( MPlotObserver * newguy) const {
	if(!observers_.contains(newguy))
		observers_.append(newguy);
}

/// get a reference to the list of subscribers
const QList<MPlotObserver*>& MPlotObservable::observers() const {
	return observers_;
}

/// remove an observer (ie: unsubscribe)
void MPlotObservable::removeObserver(MPlotObserver * removeMe) const {
	observers_.removeAll(removeMe);
}

/// send an update message to all subscribers. (use the arguments for whatever you want, but be careful ; )
void MPlotObservable::Emit(int code, const char* msg, int payload) {
	foreach(MPlotObserver* observer, observers_)
		observer->onObservableChanged(this, code, msg, payload);
}

#endif // MPLOTOBSERVABLE_H


