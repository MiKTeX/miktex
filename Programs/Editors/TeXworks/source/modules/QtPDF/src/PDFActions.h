/**
 * Copyright (C) 2013-2016  Stefan Löffler
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
#ifndef PDFActions_H
#define PDFActions_H

#include <QString>
#include <QRectF>
#include <QUrl>


#ifdef DEBUG
  #include <QDebug>
#endif

namespace QtPDF {

namespace Backend {
  class Document;
}


class PDFDestination
{
public:
  enum Type { Destination_XYZ, Destination_Fit, Destination_FitH, \
              Destination_FitV, Destination_FitR, Destination_FitB, \
              Destination_FitBH, Destination_FitBV };
  PDFDestination(const int page = -1) : _page(page), _type(Destination_XYZ), _rect(QRectF(-1, -1, -1, -1)), _zoom(-1) { }
  PDFDestination(const QString destinationName) : _page(-1), _type(Destination_XYZ), _destinationName(destinationName), _zoom(-1) { }

  bool isValid() const { return _page >= 0 || !_destinationName.isEmpty(); }
  // If the destination is not explicit (i.e., it is a named destination), use
  // Document::resolveDestination() to resolve (this must be done just-in-time
  // in case it refers to another document, or the name-to-destination mapping
  // has changed since the PDFDestination object was constructed.
  bool isExplicit() const { return _destinationName.isEmpty() && _page >= 0; }

  int page() const { return _page; }
  Type type() const { return _type; }
  QString destinationName() const { return _destinationName; }
  float zoom() const { return _zoom; }
  float top() const { return _rect.top(); }
  float left() const { return _rect.left(); }
  QRectF rect() const { return _rect; }

  // Returns the new viewport in the new page's coordinate system
  // Note: the returned viewport may have a different aspect ratio than
  // oldViewport. In that case, it view should be centered around the returned
  // rect.
  // Params:
  //  - oldViewport: viewport in old page's coordinate system
  //  - oldZoom
  QRectF viewport(const Backend::Document * doc, const QRectF oldViewport, const float oldZoom) const;
  
  void setPage(const int page) { _page = page; }
  void setType(const Type type) { _type = type; }
  void setZoom(const float zoom) { _zoom = zoom; }
  void setRect(const QRectF rect) { _rect = rect; }
  void setDestinationName(const QString destinationName) { _destinationName = destinationName; }

private:
  int _page;
  Type _type;
  QString _destinationName;
  QRectF _rect; // depending on _type, only some of the components might be significant
  float _zoom;
};

#ifdef DEBUG
  QDebug operator<<(QDebug dbg, const PDFDestination & dest);
#endif


// TODO: Possibly merge ActionTypeGoTo, ActionTypeGoToR, ActionTypeGoToE
class PDFAction
{
public:
  enum ActionType {
    ActionTypeGoTo, /*ActionTypeGoToR,*/ ActionTypeGoToE, ActionTypeLaunch,
    ActionTypeThread, ActionTypeURI, ActionTypeSound, ActionTypeMovie,
    ActionTypeHide, ActionTypeNamed, ActionTypeSubmitForm, ActionTypeResetForm,
    ActionTypeImportData, ActionTypeJavaScript, ActionTypeSetOCGState,
    ActionTypeRendition, ActionTypeTrans, ActionTypeGoTo3DView
  };

  PDFAction() { }
  virtual ~PDFAction() { }

  virtual ActionType type() const = 0;
  virtual PDFAction * clone() const = 0;
};

class PDFURIAction : public PDFAction
{
public:
  PDFURIAction(const QUrl url) : _url(url), _isMap(false) { }
  PDFURIAction(const PDFURIAction & a) : _url(a._url), _isMap(a._isMap) { }
  
  ActionType type() const { return ActionTypeURI; }
  PDFAction * clone() const { return new PDFURIAction(*this); }

  // TODO: handle _isMap (see PDF 1.7 specs)
  QUrl url() const { return _url; }

private:
  QUrl _url;
  bool _isMap;
};

class PDFGotoAction : public PDFAction
{
public:
  PDFGotoAction(const PDFDestination destination = PDFDestination()) : _destination(destination), _isRemote(false), _openInNewWindow(false) { }
  PDFGotoAction(const PDFGotoAction & a) : _destination(a._destination), _isRemote(a._isRemote), _filename(a._filename), _openInNewWindow(a._openInNewWindow) { }

  ActionType type() const { return ActionTypeGoTo; }
  PDFAction * clone() const { return new PDFGotoAction(*this); }

  PDFDestination destination() const { return _destination; }
  bool isRemote() const { return _isRemote; }
  QString filename() const { return _filename; }
  bool openInNewWindow() const { return _openInNewWindow; }

  void setDestination(const PDFDestination destination) { _destination = destination; }
  void setRemote(const bool remote = true) { _isRemote = remote; }
  void setFilename(const QString filename) { _filename = filename; }
  void setOpenInNewWindow(const bool openInNewWindow = true) { _openInNewWindow = openInNewWindow; }

private:
  PDFDestination _destination;
  bool _isRemote;
  QString _filename; // relevent only if _isRemote == true; should always refer to a PDF document (for other files, use PDFLaunchAction)
  bool _openInNewWindow; // relevent only if _isRemote == true
};

class PDFLaunchAction : public PDFAction
{
public:
  PDFLaunchAction(const QString command) : _command(command), _newWindow(false) { }

  ActionType type() const { return ActionTypeLaunch; }
  PDFAction * clone() const { return new PDFLaunchAction(*this); }
  
  QString command() const { return _command; }
  void setCommand(const QString command) { _command = command; }

  // TODO: handle newWindow, implement OS-specific extensions
private:
  QString _command;
  bool _newWindow;
};

} // namespace QtPDF

#endif // End header guard
// vim: set sw=2 ts=2 et

