#include "Utils.hpp"

#include <QStringList>

//==============================================================================
QString unscopeName(QString name)
{
  if (name.startsWith("@"))
    return name.split("%2f")[1];
  return name;
}
//==============================================================================
QString parseVersion(QString raw)
{
  if (raw == "*" || raw == "")
    return "latest";

  // Преобразование "<version1_expr> || <version2_expr> || <version3_expr>" -> "<version3_expr>"
  QString lastOfAvailableVersionsExpression = raw.split("||").takeLast().trimmed();

  // Получение версии из неравенства: ">= 1.0.0" -> "=1.0.0"
  QString availableVersion = lastOfAvailableVersionsExpression;
  if (lastOfAvailableVersionsExpression.indexOf(" - ") != -1)
    availableVersion = lastOfAvailableVersionsExpression.split(" - ").takeLast();
  else
  {
    // TODO исправить косяк со строгим равенством (< 1.0.0 == 1.0.0)
    QString withoutWhitespaces = lastOfAvailableVersionsExpression.replace(" ", "");
    int gtPos = withoutWhitespaces.indexOf(">");
    int ltPos = withoutWhitespaces.indexOf("<");
    int maxOfOperatorsPos = qMax(gtPos, ltPos);
    if (maxOfOperatorsPos > -1)
      availableVersion = withoutWhitespaces.mid(maxOfOperatorsPos + 1);
    else
      availableVersion = withoutWhitespaces;
  }

  /*
  * Приведение версии к нормализованному виду:
  * Версии имеют вид - general[-prerelease]
  * general имеют вид major[.minor[.patch]], поэтому отсутствующая часть заполняется нулями
  * Примеры:
  *   1 -> 1.0.0
  *   1.1 -> 1.1.0
  *   1.x.0 -> 1.0.0
  *   1-alptha-1-1 -> 1.0.0-alptha1-1
  */
  int sepatatorPos = availableVersion.indexOf("-");
  QString generalPart = availableVersion.left(sepatatorPos)
    .replace("x", "0")
    .replace(QRegExp("~|=|v|\\^"), "");
  QString prereleasePart = sepatatorPos == -1 ? "" : availableVersion.mid(sepatatorPos);
  return generalPart + QString(".0").repeated(3 - generalPart.split(".").size()) + prereleasePart;
}
//==============================================================================
