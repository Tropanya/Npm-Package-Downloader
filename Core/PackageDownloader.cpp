#include "PackageDownloader.hpp"

#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>

#include <QDir>
#include <QFile>
#include <QDataStream>

#include "Utils.hpp"

//==============================================================================
PackageDownloader::PackageDownloader(QString registryApi, QString outputDir,
                                     QObject* parent)
  : QObject(parent)
  , mRegistryApi(registryApi)
  , mOutputDir(outputDir)
  , mPackagesInProgress(0)
{
  connect(&mFindPackageManager, &QNetworkAccessManager::finished,
           this,                &PackageDownloader::findPackageFinish);

  connect(&mDownloadPackageManager, &QNetworkAccessManager::finished,
           this,                    &PackageDownloader::downloadPackageFinish);
}
//==============================================================================
void PackageDownloader::findPackage(QString name, QString version)
{
  QString packageName = name.replace("/", "%2f");

  PackageDataPtr packageData;
  if (mPackages.find(packageName) == mPackages.end())
  {
    packageData = std::make_shared<PackageData>(packageName);
    mPackages.insert({ packageName, packageData });
  }
  else
  {
    packageData = mPackages[packageName];
  }

  std::set<QString>& versions = packageData.get()->versions;
  if (versions.find(version) != versions.end())
    return;

  QString urlString;
  if (version == "latest")
    urlString = QString("%1/%2/").arg(mRegistryApi, packageName);
  else
    urlString = QString("%1/%2/%3").arg(mRegistryApi, packageName, version);

  mPackagesInProgress++;
  QNetworkRequest request;
  request.setUrl(QUrl(urlString));
  mFindPackageManager.get(request);
}
//==============================================================================
void PackageDownloader::downloadPackages()
{
  QNetworkRequest request;
  for (const auto& package : mPackages)
  {
    PackageData* packageData = package.second.get();
    for (const QString packageVersion : packageData->versions)
    {
      QString unscopedPackageName = unscopeName(packageData->name);
      QString urlString = QString("%1/%2/-/%3-%4.tgz").arg(mRegistryApi,
                                                           packageData->name,
                                                           unscopedPackageName,
                                                           packageVersion);
      mPackagesInProgress++;
      request.setUrl(QUrl(urlString));
      mDownloadPackageManager.get(request);
    }
  }
}
//==============================================================================
void PackageDownloader::findPackageFinish(QNetworkReply* reply)
{
  if (reply->error() != QNetworkReply::NoError)
  {
    int statusCode =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "[ERROR]" << statusCode
             << "Failed to found package" << "|" << reply->errorString();
    mPackagesInProgress--;
    return;
  }

  QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
  QJsonObject data = jsonResponse.object();

  QString url = reply->url().toString();
  QString nameAndVerionStr = url.split(mRegistryApi + "/")[1];
  QStringList nameAndVerion = nameAndVerionStr.split("/");
  PackageInfo info = {
    nameAndVerion[0].replace("%2F", "%2f"),
    nameAndVerion[1]
  };

  PackageDataPtr packageData = mPackages[info.name];
  QJsonValue dependencies;
  if (info.version == "")
  {
    QJsonObject distTags = data.take("dist-tags").toObject();
    QString version = distTags.take("latest").toString();
    packageData.get()->versions.insert("latest");
    packageData.get()->versions.insert(version);

    QJsonValue versions = data.take("versions");
    if (versions == QJsonValue::Undefined)
      dependencies = QJsonValue::Undefined;
    else
    {
      QJsonObject versionsData = versions.toObject();
      QJsonObject versionData = versionsData.value(version).toObject();
      dependencies = versionData.take("dependencies");
    }
  }
  else
  {
    dependencies = data.take("dependencies");
    packageData.get()->versions.insert(info.version);
  }

  std::set<std::pair<QString, QString>> requairedPackages;
  if (dependencies != QJsonValue::Undefined)
  {
    QJsonObject dependenciesData = dependencies.toObject();
    for (QString dependency : dependenciesData.keys())
    {
       QString dependencyVersion
         = parseVersion(dependenciesData.take(dependency).toString());
       requairedPackages.insert({ dependency, dependencyVersion });
    }
  }

  for (const std::pair<QString, QString>& requairedPackage : requairedPackages)
    findPackage(requairedPackage.first, requairedPackage.second);
  mPackagesInProgress--;

  if (!mPackagesInProgress)
  {
    for (auto package : mPackages)
    {
      PackageData* const packageData = package.second.get();
      packageData->versions.erase("latest");
    }

    qDebug() << "FINISH FINDING";
    downloadPackages(); // tmp
  }
}
//=============================================================================
void PackageDownloader::downloadPackageFinish(QNetworkReply* reply)
{
  if (reply->error() != QNetworkReply::NoError)
  {
    int statusCode =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "[ERROR]" << statusCode
             << "Failed to load package" << "|" << reply->errorString();
    mPackagesInProgress--;
    return;
  }

  QString url = reply->url().toString();
  QString filePath = QString("%1/%2").arg(mOutputDir,
                                          url.split(mRegistryApi + "/")[1]);
  filePath.replace("/-/", "/").replace("%2F", "/").replace("%2f", "/");
  QDir dir = QFileInfo(filePath).absoluteDir();
  if (!dir.exists())
    QDir().mkpath(dir.absolutePath());

  QFile file(filePath);
  file.open(QFile::WriteOnly);
  QDataStream stream(&file);
  stream.setVersion(QDataStream::Qt_5_5);
  stream << reply->readAll();
  file.close();

  mPackagesInProgress--;
  if (!mPackagesInProgress)
    qDebug() << "FINISH DOWNLOADING";
}
//=============================================================================
