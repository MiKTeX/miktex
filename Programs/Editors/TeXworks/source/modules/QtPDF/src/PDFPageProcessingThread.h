/**
 * Copyright (C) 2023-2024  Stefan Löffler
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */
#ifndef PDFPageProcessingThread_H
#define PDFPageProcessingThread_H

#include <QEvent>
#include <QImage>
#include <QMutex>
#include <QObject>
#include <QRect>
#include <QStack>
#include <QThread>
#include <QWaitCondition>

namespace QtPDF {

namespace Annotation {
class Link;
} // namespace Annotation

namespace Backend {

class Page;

class PageProcessingRequest : public QObject
{
  Q_OBJECT
  friend class PDFPageProcessingThread;

  // Protect c'tor and execute() so we can't access them except in derived
  // classes and friends
protected:
  PageProcessingRequest(Page *page, QObject *listener) : page(page), listener(listener) { }
  // Should perform whatever processing it is designed to do
  // Returns true if finished successfully, false otherwise
  virtual bool execute() = 0;

public:
  enum Type { PageRendering, LoadLinks };

  ~PageProcessingRequest() override = default;
  virtual Type type() const = 0;

  Page *page;
  QObject *listener;

  virtual bool operator==(const PageProcessingRequest & r) const;
#ifdef DEBUG
  virtual operator QString() const = 0;
#endif
};

class PageProcessingRenderPageRequest : public PageProcessingRequest
{
  Q_OBJECT
  friend class PDFPageProcessingThread;

public:
  PageProcessingRenderPageRequest(Page *page, QObject *listener, double xres, double yres, QRect render_box = QRect(), bool cache = false) :
    PageProcessingRequest(page, listener),
    xres(xres), yres(yres),
    render_box(render_box),
    cache(cache)
  {}
  Type type() const override { return PageRendering; }

  bool operator==(const PageProcessingRequest & r) const override;
#ifdef DEBUG
  operator QString() const override;
#endif

protected:
  bool execute() override;

  double xres, yres;
  QRect render_box;
  bool cache;
};


class PDFPageRenderedEvent : public QEvent
{

public:
  PDFPageRenderedEvent(double xres, double yres, QRect render_rect, QImage rendered_page):
    QEvent(PageRenderedEvent),
    xres(xres), yres(yres),
    render_rect(render_rect),
    rendered_page(rendered_page)
  {}

  static const QEvent::Type PageRenderedEvent;

  const double xres, yres;
  const QRect render_rect;
  const QImage rendered_page;

};


class PageProcessingLoadLinksRequest : public PageProcessingRequest
{
  Q_OBJECT
  friend class PDFPageProcessingThread;

public:
  PageProcessingLoadLinksRequest(Page *page, QObject *listener) : PageProcessingRequest(page, listener) { }
  Type type() const override { return LoadLinks; }

#ifdef DEBUG
  operator QString() const override;
#endif

protected:
  bool execute() override;
};


class PDFLinksLoadedEvent : public QEvent
{

public:
  PDFLinksLoadedEvent(const QList< QSharedPointer<Annotation::Link> > links):
    QEvent(LinksLoadedEvent),
    links(links)
  {}

  static const QEvent::Type LinksLoadedEvent;

  const QList< QSharedPointer<Annotation::Link> > links;

};


// Class to perform (possibly) lengthy operations on pages in the background
// Modelled after the "Blocking Fortune Client Example" in the Qt docs
// (http://doc.qt.nokia.com/stable/network-blockingfortuneclient.html)

// The `PDFPageProcessingThread` is a thread that processes background jobs.
// Each job is represented by a subclass of `PageProcessingRequest` and
// contains an `execute` method that performs the actual work.
class PDFPageProcessingThread : public QThread
{
  Q_OBJECT

public:
  PDFPageProcessingThread() = default;
  ~PDFPageProcessingThread() override;

  // add a processing request to the work stack
  // Note: request must have been created on the heap and must be in the scope
  // of this thread; use requestRenderPage() and requestLoadLinks() for that
  void addPageProcessingRequest(PageProcessingRequest * request);

  // drop all remaining processing requests
  // WARNING: This function *must not* be called while the calling thread holds
  // any locks that would prevent and work item from finishing. Otherwise, we
  // could run into the following deadlock scenario:
  // clearWorkStack() waits for the currently active work items to finish. The
  // currently active work item waits to acquire a lock necessary for it to
  // finish. However, that lock is held by the caller of clearWorkStack().
  void clearWorkStack();

protected:
  void run() override;

private:
  QStack<PageProcessingRequest*> _workStack;
  QMutex _mutex;
  QWaitCondition _waitCondition;
  bool _idle{true};
  QWaitCondition _idleCondition;
  bool _quit{false};
#ifdef DEBUG
  static void dumpWorkStack(const QStack<PageProcessingRequest*> & ws);
#endif

};

} // namespace Backend
} // namespace QtPDF

#endif // PDFPageProcessingThread_H
