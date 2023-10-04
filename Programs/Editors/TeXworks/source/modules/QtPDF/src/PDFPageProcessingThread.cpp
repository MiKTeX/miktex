#include "PDFPageProcessingThread.h"

#include "PDFBackend.h"

#include <QCoreApplication>

namespace QtPDF {
namespace Backend {

#ifdef DEBUG
void PDFPageProcessingThread::dumpWorkStack(const QStack<PageProcessingRequest*> & ws)
{
  QStringList strList;
  for (int i = 0; i < ws.size(); ++i) {
    PageProcessingRequest * request = ws[i];
    if (!request)
      strList << QString::fromUtf8("NULL");
    else {
      strList << *request;
    }
  }
  qDebug() << strList;
}
#endif

// Backend Rendering
// =================

PDFPageProcessingThread::~PDFPageProcessingThread()
{
  _mutex.lock();
  _quit = true;
  _waitCondition.wakeAll();
  _mutex.unlock();
  wait();
}

void PDFPageProcessingThread::addPageProcessingRequest(PageProcessingRequest * request)
{

  if (!request)
    return;

  // `request` must live in the main (GUI) thread, or else destroying it later
  // on will fail
  Q_ASSERT(request->thread() == QCoreApplication::instance()->thread());

  QMutexLocker locker(&(this->_mutex));
  // Note: Commenting the "remove identical requests in the stack" code for now.
  // This should be handled by the caching routine elsewhere automatically. If
  // in doubt, it's better to render a tile twice than to not render it at all
  // (thereby leaving the dummy image in the cache indefinitely)
/*
  // remove any instances of the given request type before adding the new one to
  // avoid processing it several times
  // **TODO:** Could it be that we require several concurrent versions of the
  //           same page?
  int i;
  for (i = _workStack.size() - 1; i >= 0; --i) {
    if (*(_workStack[i]) == *request) {
      // Using deleteLater() doesn't work because we have no event queue in this
      // thread. However, since the object is still on the stack, it is still
      // sleeping and directly deleting it should therefore be safe.
      delete _workStack[i];
      _workStack.remove(i);
    }
  }
*/

  _workStack.push(request);
#ifdef DEBUG
  qDebug() << "new request:" << *request;
#endif

  locker.unlock();
  if (!isRunning())
    start();
  else
    _waitCondition.wakeOne();
}

void PDFPageProcessingThread::run()
{
  _mutex.lock();
  _idle = false;
  while (!_quit) {
    // mutex must be locked at start of loop
    if (!_workStack.empty()) {
      PageProcessingRequest * workItem = _workStack.pop();
      _mutex.unlock();

#ifdef DEBUG
      qDebug() << "processing work item" << *workItem << "; remaining items:" << _workStack.size();
      QElapsedTimer timer;
      timer.start();
#endif
      workItem->execute();
#ifdef DEBUG
      QString jobDesc;
      switch (workItem->type()) {
        case PageProcessingRequest::LoadLinks:
          jobDesc = QString::fromUtf8("loading links");
          break;
        case PageProcessingRequest::PageRendering:
          jobDesc = QString::fromUtf8("rendering page");
          break;
      }
      qDebug() << "finished " << jobDesc << "for page" << workItem->page->pageNum() << ". Time elapsed: " << timer.elapsed() << " ms.";
#endif

      // Delete the work item as it has fulfilled its purpose
      // Note that we can't delete it here or we might risk that some emitted
      // signals are invalidated; to ensure they reach their destination, we
      // need to call deleteLater().
      // Note: workItem *must* live in the main (GUI) thread for this!
      Q_ASSERT(workItem->thread() == QCoreApplication::instance()->thread());
      workItem->deleteLater();

      _mutex.lock();
    }
    else {
#ifdef DEBUG
      qDebug() << "going to sleep";
#endif
      _idle = true;
      _idleCondition.wakeAll();
      _waitCondition.wait(&_mutex);
      _idle = false;
#ifdef DEBUG
      qDebug() << "waking up";
#endif
    }
  }
  _mutex.unlock();
}

void PDFPageProcessingThread::clearWorkStack()
{
  _mutex.lock();

  foreach(PageProcessingRequest * workItem, _workStack) {
    if (!workItem)
      continue;
    Q_ASSERT(workItem->thread() == QCoreApplication::instance()->thread());
    workItem->deleteLater();
  }
  _workStack.clear();

  if (!_idle) {
    // Wait until the current operation finishes
    _idleCondition.wait(&_mutex);
  }
  _mutex.unlock();
}


// Asynchronous Page Operations
// ----------------------------
//
// The `execute` functions here are called by the processing theread to perform
// background jobs such as page rendering or link loading. This alows the GUI
// thread to stay unblocked and responsive. The results of background jobs are
// posted as events to a `listener` which can be any subclass of `QObject`. The
// `listener` will need a custom `event` function that is capable of picking up
// on these events.

bool PageProcessingRequest::operator==(const PageProcessingRequest & r) const
{
  // TODO: Should we care about the listener here as well?
  return (type() == r.type() && page == r.page);
}

bool PageProcessingRenderPageRequest::operator==(const PageProcessingRequest & r) const
{
  if (!PageProcessingRequest::operator==(r))
    return false;
  const PageProcessingRenderPageRequest * rr = dynamic_cast<const PageProcessingRenderPageRequest*>(&r);
  // TODO: Should we care about the listener here as well?
  return (qFuzzyCompare(xres, rr->xres) && qFuzzyCompare(yres, rr->yres) && render_box == rr->render_box && cache == rr->cache);
}

#ifdef DEBUG
PageProcessingRenderPageRequest::operator QString() const
{
  return QString::fromUtf8("RP:%1.%2_%3").arg(page->pageNum()).arg(render_box.topLeft().x()).arg(render_box.topLeft().y());
}
#endif

// ### Custom Event Types
// These are the events posted by `execute` functions.
const QEvent::Type PDFPageRenderedEvent::PageRenderedEvent = static_cast<QEvent::Type>( QEvent::registerEventType() );
const QEvent::Type PDFLinksLoadedEvent::LinksLoadedEvent = static_cast<QEvent::Type>( QEvent::registerEventType() );

bool PageProcessingRenderPageRequest::execute()
{
  // TODO: Aborting renders doesn't really work right now---the backend knows
  // nothing about the PDF scenes.
  //
  // Idea: Perhaps allow page render requests to provide a pointer to a function
  // that returns a `bool` value indicating if the request is still valid? Then
  // the `PDFPageGraphicsItem` could have a function that indicates if the item
  // is anywhere near a viewport.
  QImage rendered_page = page->renderToImage(xres, yres, render_box, cache);
  QCoreApplication::postEvent(listener, new PDFPageRenderedEvent(xres, yres, render_box, rendered_page));

  return true;
}

bool PageProcessingLoadLinksRequest::execute()
{
  QCoreApplication::postEvent(listener, new PDFLinksLoadedEvent(page->loadLinks()));
  return true;
}

#ifdef DEBUG
PageProcessingLoadLinksRequest::operator QString() const
{
  return QString::fromUtf8("LL:%1").arg(page->pageNum());
}
#endif

} // namespace Backend
} // namespace QtPDF
