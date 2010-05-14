#ifndef MPLOTOBSERVABLE_H
#define MPLOTOBSERVABLE_H

#include "MPlotObserver.h"
#include <QList>

class MPlotObservable {

	/// MPlotObservable is a base class that can be inherited to provide a standard interface for notifying MPlotObserver's of changes to its state.  It's not as safe or as powerful as Qt's signals and slots, but it avoids the problem with the inability to do multiple-inheritance of QObjects.
public:
	MPlotObservable() {}

public:

	virtual ~MPlotObservable() {}

  /// connect an observer (ie: subscribe)
  void addObserver ( MPlotObserver * newguy) const {
	  if(!observers_.contains(newguy))
		  observers_ << newguy;
  }

  /// get a reference to the list of subscribers
  const QList<MPlotObserver*>& observers() const { return observers_; }

  /// remove an observer (ie: unsubscribe)
  void removeObserver(MPlotObserver * removeMe) const {
	  observers_.removeAll(removeMe);
  }

  /// send an update message to all subscribers. (use the arguments for whatever you want, but be careful ; )
  virtual void Emit(int code, const char* msg, int payload = 0) {
	  foreach(MPlotObserver* observer, observers_)
		  observer->onObservableChanged(this, code, msg, payload);
  }

protected:

  mutable QList<MPlotObserver*> observers_;

};



#endif // MPLOTOBSERVABLE_H

