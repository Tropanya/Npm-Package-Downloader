#ifndef PACKAGE_DOWNLOADER_HPP
#define PACKAGE_DOWNLOADER_HPP

#include <QObject>
#include <QNetworkAccessManager>

#include <set>
#include <map>
#include <memory>

class QNetworkReply;

struct PackageInfo
{
  QString name;
  QString version;
};

struct PackageData
{
  QString name;
  std::set<QString> versions;
  PackageData(QString packageName) : name(packageName) {}
};
typedef std::shared_ptr<PackageData> PackageDataPtr;
typedef std::map<QString, PackageDataPtr> PackagesData;

class PackageDownloader:
  public QObject
{
  Q_OBJECT
public:
  PackageDownloader(QString registryApi, QString outputDir,
                QObject* parent = nullptr);
  void findPackage(QString name, QString version = "latest");
  void downloadPackages();
private:
  Q_SLOT void findPackageFinish(QNetworkReply* reply);
  Q_SLOT void downloadPackageFinish(QNetworkReply* reply);
private:
  QString mRegistryApi;
  QString mOutputDir;

  QNetworkAccessManager mFindPackageManager;
  QNetworkAccessManager mDownloadPackageManager;

  PackagesData mPackages;
  std::size_t mPackagesInProgress;
};

#endif // !PACKAGE_DOWNLOADER_HPP
