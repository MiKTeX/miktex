#ifndef SpellChecker_H
#define SpellChecker_H

#include <memory>
#include <QString>
#include <QStringList>
#include <QTextCodec>
#include <vector>

struct Hunhandle;

namespace Tw {
namespace Document {

class SpellChecker {
	using DictType = std::shared_ptr<Hunhandle>;

	struct DictRef {
		QString language;
		mutable std::weak_ptr<Hunhandle> hunhandle;
		QTextCodec * codec{QTextCodec::codecForLocale()};

		bool operator==(const DictRef & other) const;
		operator bool() const { return isValid(); }
		bool isValid() const;
		std::shared_ptr<Hunhandle> getHunhandle() const;
	};

	std::vector<DictRef> m_dicts;

public:
	SpellChecker() = default;

	SpellChecker(const QString & language);

	bool operator==(const SpellChecker & other) const;
	bool operator!=(const SpellChecker & other) const { return !operator==(other); }
	operator bool() const { return isValid(); }

	bool isValid() const;

	QStringList languages() const;
	bool setLanguages(const QStringList & languages);
	bool isWordCorrect(const QString & word) const;
	QList<QString> suggestionsForWord(const QString & word) const;
	// note that this is not persistent after quitting TW
	void ignoreWord(const QString & word);
};

} // namespace Document
} // namespace Tw

#endif // SpellChecker_H
