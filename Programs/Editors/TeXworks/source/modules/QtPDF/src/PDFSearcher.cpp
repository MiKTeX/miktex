/**
 * Copyright (C) 2022  Stefan Löffler
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
#include "PDFSearcher.h"

void QtPDF::PDFSearcher::populatePages()
{
  const QMutexLocker mutexLocker{&m_mutex};
  m_pages.clear();
  const QSharedPointer<Backend::Document> doc{m_doc.toStrongRef()};

  if (!doc) {
    return;
  }

  const int numPages = doc->numPages();
  const int startPage = [=] () {
    if (m_startPage >= 0 && m_startPage < numPages) {
      return m_startPage;
    }
    return 0;
  }();

  if (!m_searchFlags.testFlag(Backend::Search_Backwards))
  {
    // Search forwards
    for (int page = startPage; page < numPages; ++page) {
      m_pages.append(page);
    }
    if (m_searchFlags.testFlag(Backend::Search_WrapAround)) {
      for (int page = 0; page < startPage; ++page) {
        m_pages.append(page);
      }
    }
  }
  else {
    // Search backwards
    for (int page = startPage; page >= 0; --page) {
      m_pages.append(page);
    }
    if (m_searchFlags.testFlag(Backend::Search_WrapAround)) {
      for (int page = numPages - 1; page > startPage; --page) {
        m_pages.append(page);
      }
    }
  }
}

void QtPDF::PDFSearcher::ensureStopped()
{
  if (!isRunning()) {
    return;
  }
  requestInterruption();
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
  wait(ULONG_MAX);
#else
  wait();
#endif
}

void QtPDF::PDFSearcher::clear()
{
  const QMutexLocker mutexLocker{&m_mutex};
  m_pages.clear();
  m_results.clear();
}

void QtPDF::PDFSearcher::stopAndClear()
{
  ensureStopped();
  clear();
}

QString QtPDF::PDFSearcher::searchString() const
{
  const QMutexLocker mutexLocker{&m_mutex};
  return m_searchString;
}

void QtPDF::PDFSearcher::setSearchString(const QString &searchString)
{
  stopAndClear();
  const QMutexLocker mutexLocker{&m_mutex};
  m_searchString = searchString;
}

QtPDF::Backend::SearchFlags QtPDF::PDFSearcher::searchFlags() const
{
  const QMutexLocker mutexLocker{&m_mutex};
  return m_searchFlags;
}

void QtPDF::PDFSearcher::setSearchFlags(const Backend::SearchFlags &flags)
{
  stopAndClear();
  const QMutexLocker mutexLocker{&m_mutex};
  m_searchFlags = flags;
}

QWeakPointer<QtPDF::Backend::Document> QtPDF::PDFSearcher::document() const
{
  const QMutexLocker mutexLocker{&m_mutex};
  return m_doc;
}

void QtPDF::PDFSearcher::setDocument(const QWeakPointer<Backend::Document> &doc)
{
  stopAndClear();
  const QMutexLocker mutexLocker{&m_mutex};
  m_doc = doc;
}

int QtPDF::PDFSearcher::startPage() const
{
  const QMutexLocker mutexLocker{&m_mutex};
  return m_startPage;
}

void QtPDF::PDFSearcher::setStartPage(const int page)
{
  const QMutexLocker mutexLocker{&m_mutex};
  m_startPage = page;
}

int QtPDF::PDFSearcher::progressValue() const
{
  const QMutexLocker mutexLocker{&m_mutex};
  int retVal{0};
  for (const SearchResult & result : m_results) {
    if (result.finished) {
      ++retVal;
    }
  }
  return retVal;
}

int QtPDF::PDFSearcher::progressMaximum() const
{
  const QMutexLocker mutexLocker{&m_mutex};
  return m_pages.size();
}

QList<QtPDF::Backend::SearchResult> QtPDF::PDFSearcher::resultAt(int page) const
{
  const QMutexLocker mutexLocker{&m_mutex};
  if (page < 0 || page >= m_results.size()) {
    return {};
  }
  return m_results.at(page).occurences;
}

void QtPDF::PDFSearcher::run()
{
  clear();
  populatePages();

  const QSharedPointer<Backend::Document> doc = [this] () {
    const QMutexLocker mutexLocker{&m_mutex};
    return m_doc.toStrongRef();
  }();
  if (!doc) {
    return;
  }

  {
    const QMutexLocker mutexLocker{&m_mutex};
    m_results.resize(doc->numPages());
  }

  for (const int & pageIndex : m_pages) {
    if (isInterruptionRequested()) {
      break;
    }
    const QSharedPointer<QtPDF::Backend::Page> page{doc->page(pageIndex).toStrongRef()};
    if (!page) {
      continue;
    }
    auto result = page->search(m_searchString, m_searchFlags);
    {
      const QMutexLocker mutexLocker{&m_mutex};
      m_results[pageIndex].occurences = std::move(result);
      m_results[pageIndex].finished = true;
    }
    emit resultReady(pageIndex);
    emit progressValueChanged(progressValue());
  }
}
