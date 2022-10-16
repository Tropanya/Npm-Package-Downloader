#include <QCoreApplication>

#include "Core/PackageDownloader.hpp"

int main(int argc, char** argv)
{
  QCoreApplication app(argc, argv);

  QString registryApi = "https://registry.npmjs.org";
  QString outputDir = "./output";

  PackageDownloader downloader(registryApi, outputDir);
  downloader.findPackage("react");

  return app.exec();
}
