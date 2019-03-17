#ifndef ENGINE_H
#define ENGINE_H

#include <QObject>
#include <QProcess>
#include <QFileInfo>

// specification of an "engine" used to process files
class Engine : public QObject
{
	Q_OBJECT

public:
	Engine();
	Engine(const QString& name, const QString& program, const QStringList arguments, bool showPdf);
	Engine(const Engine& orig);
	Engine& operator=(const Engine& rhs);

	const QString name() const;
	const QString program() const;
	const QStringList arguments() const;
	bool showPdf() const;

	void setName(const QString& name);
	void setProgram(const QString& program);
	void setArguments(const QStringList& arguments);
	void setShowPdf(bool showPdf);

	bool isAvailable() const;
	QProcess * run(QFileInfo input, QObject * parent = nullptr);

	static QStringList binPaths();

private:
	static QString programPath(const QString & prog);

	QString _name;
	QString _program;
	QStringList _arguments;
	bool _showPdf;
};



#endif // !defined(ENGINE_H)
