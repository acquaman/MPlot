#ifndef MPLOTOBSERVABLE_H
#define MPLOTOBSERVABLE_H

#include "MPlotObserver.h"
#include <QList>

class MPlotObservable {

	/// MPlotObservable is a base class that can be inherited to provide a standard interface for notifying MPlotObserver's of changes to its state.  It's not as safe or as powerful as Qt's signals and slots, but it avoids the problem with the inability to do multiple-inheritance of QObjects.
public:
	MPlotObservable();

public:

	virtual ~MPlotObservable();

	/// connect an observer (ie: subscribe)
	void addObserver ( MPlotObserver * newguy) const;

	/// get a reference to the list of subscribers
	const QList<MPlotObserver*>& observers() const;

	/// remove an observer (ie: unsubscribe)
	void removeObserver(MPlotObserver * removeMe) const;

	/// send an update message to all subscribers. (use the arguments for whatever you want, but be careful ; )
	virtual void Emit(int code, const char* msg, int payload = 0);

protected:

	mutable QList<MPlotObserver*> observers_;

};



#endif // MPLOTOBSERVABLE_H

