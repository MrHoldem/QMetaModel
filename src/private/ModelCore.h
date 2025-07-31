#ifndef QFORGE_MODELCORE_HPP
#define QFORGE_MODELCORE_HPP

#include <QString>
#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <memory>

#include "QueryHandler.hpp"
#include "QueryResult.hpp"
#include "ModelSchema.h"

namespace QForge::nsModel
{

class ModelCore
{
public:
    /*!
     * \brief Конструктор.
     * \param configPath Путь к конфигу.
     * \param handler Обработчик обращений к БД.
     */
    explicit ModelCore(const QString& configPath, const QueryHandler& handler);

    /*!
     * \brief Конструктор с готовой схемой.
     * \param schema Готовая схема модели.
     * \param handler Обработчик обращений к БД.
     */
    explicit ModelCore(const ModelSchema& schema, const QueryHandler& handler);

    /*!
     * \brief Проверяет, корректно ли был считан файл конфигурации.
     * \return Флаг успеха.
     */
    bool isValid() const;

    /*!
     * \brief Запускает выполнение запроса.
     * \param queryId Идентификатор (имя) запроса.
     * \param params Параметры.
     * \return Результат выполнения запроса.
     */
    QueryResult execute(const QString& queryId, const QVariantMap& params = {}) const;

    /*!
     * \brief Возвращает схему модели.
     * \return Схема модели.
     */
    const ModelSchema& getSchema() const;

    /*!
     * \brief Возвращает схему модели в виде JSON.
     * \return JSON-схема.
     */
    QJsonObject getJsonSchema() const;

    /*!
     * \brief Возвращает список доступных запросов.
     * \return Список запросов (id).
     */
    QStringList getQueriesList() const;

    /*!
     * \brief Перезагружает данные конфигурации.
     * \return Флаг успеха.
     */
    bool reload();

    /*!
     * \brief Возвращает список ошибок.
     * \return Список ошибок.
     */
    const QStringList& getErrors() const;

    /*!
     * \brief Очищает список ошибок.
     */
    void clearErrors();

private:
    /*!
     * \brief Загружает данные из заданного файла конфигурации.
     * \return Флаг успеха.
     */
    bool loadFromFile();

    /*!
     * \brief Парсит YAML содержимое в схему.
     * \param content YAML содержимое.
     * \return Флаг успеха.
     */
    bool parseYaml(const QString& content);

    /*!
     * \brief Парсит JSON содержимое в схему.
     * \param content JSON содержимое.
     * \return Флаг успеха.
     */
    bool parseJson(const QString& content);

    /*!
     * \brief Валидирует параметры запроса.
     * \param queryId Идентификатор запроса.
     * \param params Параметры.
     * \return Результат валидации.
     */
    QueryResult validateQueryParams(const QString& queryId, const QVariantMap& params) const;

private:
    QStringList errors_log;
    QueryHandler handler;
    QString path;
    bool isValidFlag = false;
    std::unique_ptr<ModelSchema> schema;
};

}

#endif // QFORGE_MODELCORE_HPP
