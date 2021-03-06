#import <Foundation/NSProcessInfo.h>
#include <QString>

QString GetMacOSVersionString() {
	auto osv = [[NSProcessInfo processInfo] operatingSystemVersion];
	return QStringLiteral("Mac OS X %1.%2.%3").arg(osv.majorVersion).arg(osv.minorVersion).arg(osv.patchVersion);
}
