#include "TimelineModel.h"
#include <spdlog/spdlog.h>
#include <QUuid>
#include <algorithm>

namespace suno::ui {

TimelineModel::TimelineModel(QObject* parent)
    : QAbstractListModel(parent)
{
    spdlog::debug("TimelineModel created");
}

int TimelineModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_entries.size();
}

QVariant TimelineModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size()) {
        return QVariant();
    }
    
    const TimelineEntry& entry = m_entries[index.row()];
    
    switch (role) {
        case IdRole: return entry.id;
        case TypeRole: return entry.type;
        case NameRole: return entry.name;
        case DescriptionRole: return entry.description;
        case CommitHashRole: return entry.gitCommitHash;
        case PositionRole: return entry.positionMs;
        case DurationRole: return entry.durationMs;
        case TimestampRole: return entry.timestamp;
        case IsActiveRole: return entry.isActive;
        case PropertiesRole: return entry.properties;
        default: return QVariant();
    }
}

QHash<int, QByteArray> TimelineModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "entryId";
    roles[TypeRole] = "entryType";
    roles[NameRole] = "name";
    roles[DescriptionRole] = "description";
    roles[CommitHashRole] = "commitHash";
    roles[PositionRole] = "position";
    roles[DurationRole] = "duration";
    roles[TimestampRole] = "timestamp";
    roles[IsActiveRole] = "isActive";
    roles[PropertiesRole] = "properties";
    return roles;
}

void TimelineModel::addEntry(const QString& type,
                              const QString& name,
                              qint64 positionMs,
                              qint64 durationMs)
{
    TimelineEntry entry;
    entry.id = generateId();
    entry.type = type;
    entry.name = name;
    entry.positionMs = positionMs;
    entry.durationMs = durationMs;
    entry.timestamp = QDateTime::currentDateTime();
    entry.isActive = true;
    
    // Find insertion point to maintain sorted order by position
    int insertIndex = 0;
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].positionMs > positionMs) {
            break;
        }
        insertIndex = i + 1;
    }
    
    beginInsertRows(QModelIndex(), insertIndex, insertIndex);
    m_entries.insert(insertIndex, entry);
    endInsertRows();
    
    emit countChanged();
    emit entryAdded(entry.id);
    
    spdlog::debug("Added timeline entry: {} at {}ms", name.toStdString(), positionMs);
}

void TimelineModel::removeEntry(const QString& id)
{
    int index = findEntryIndex(id);
    if (index < 0) {
        spdlog::warn("Entry not found: {}", id.toStdString());
        return;
    }
    
    beginRemoveRows(QModelIndex(), index, index);
    m_entries.removeAt(index);
    endRemoveRows();
    
    emit countChanged();
    emit entryRemoved(id);
    
    spdlog::debug("Removed timeline entry: {}", id.toStdString());
}

void TimelineModel::updateEntry(const QString& id, const QVariantMap& properties)
{
    int index = findEntryIndex(id);
    if (index < 0) {
        return;
    }
    
    TimelineEntry& entry = m_entries[index];
    
    if (properties.contains("name")) {
        entry.name = properties["name"].toString();
    }
    if (properties.contains("description")) {
        entry.description = properties["description"].toString();
    }
    if (properties.contains("isActive")) {
        entry.isActive = properties["isActive"].toBool();
    }
    if (properties.contains("duration")) {
        entry.durationMs = properties["duration"].toLongLong();
    }
    
    // Merge custom properties
    for (auto it = properties.begin(); it != properties.end(); ++it) {
        if (!QStringList{"name", "description", "isActive", "duration"}.contains(it.key())) {
            entry.properties[it.key()] = it.value();
        }
    }
    
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);
    emit entryModified(id);
}

void TimelineModel::moveEntry(const QString& id, qint64 newPositionMs)
{
    int oldIndex = findEntryIndex(id);
    if (oldIndex < 0) {
        return;
    }
    
    TimelineEntry entry = m_entries[oldIndex];
    entry.positionMs = newPositionMs;
    
    // Find new position in sorted order
    int newIndex = 0;
    for (int i = 0; i < m_entries.size(); ++i) {
        if (i == oldIndex) continue;
        if (m_entries[i].positionMs > newPositionMs) {
            break;
        }
        newIndex = i + 1;
    }
    
    if (newIndex > oldIndex) {
        newIndex--;
    }
    
    if (newIndex != oldIndex) {
        beginMoveRows(QModelIndex(), oldIndex, oldIndex, QModelIndex(), 
                      newIndex > oldIndex ? newIndex + 1 : newIndex);
        m_entries.removeAt(oldIndex);
        m_entries.insert(newIndex, entry);
        endMoveRows();
    } else {
        m_entries[oldIndex] = entry;
        QModelIndex modelIndex = createIndex(oldIndex, 0);
        emit dataChanged(modelIndex, modelIndex);
    }
    
    emit entryModified(id);
}

void TimelineModel::clear()
{
    if (m_entries.isEmpty()) {
        return;
    }
    
    beginResetModel();
    m_entries.clear();
    endResetModel();
    
    emit countChanged();
}

void TimelineModel::commitEntry(const QString& id, const QString& message)
{
    int index = findEntryIndex(id);
    if (index < 0) {
        return;
    }
    
    // TODO: Integrate with GitController
    spdlog::info("Committing entry '{}': {}", id.toStdString(), message.toStdString());
    
    // Generate a mock commit hash for now
    QString commitHash = QUuid::createUuid().toString(QUuid::Id128).left(8);
    m_entries[index].gitCommitHash = commitHash;
    
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);
}

void TimelineModel::revertEntry(const QString& id, const QString& commitHash)
{
    spdlog::info("Reverting entry '{}' to commit {}", 
                 id.toStdString(), commitHash.toStdString());
    
    // TODO: Integrate with GitController to restore entry state
}

QVariantList TimelineModel::getEntryHistory(const QString& id) const
{
    // TODO: Integrate with GitController
    Q_UNUSED(id)
    return QVariantList();
}

QVariantList TimelineModel::entriesAtPosition(qint64 positionMs) const
{
    QVariantList result;
    
    for (const TimelineEntry& entry : m_entries) {
        if (!entry.isActive) continue;
        
        qint64 entryEnd = entry.positionMs + entry.durationMs;
        
        if (positionMs >= entry.positionMs && positionMs < entryEnd) {
            QVariantMap entryMap;
            entryMap["id"] = entry.id;
            entryMap["type"] = entry.type;
            entryMap["name"] = entry.name;
            entryMap["position"] = entry.positionMs;
            entryMap["duration"] = entry.durationMs;
            result.append(entryMap);
        }
    }
    
    return result;
}

QString TimelineModel::activePresetAtPosition(qint64 positionMs) const
{
    // Find the last preset that started before or at this position
    QString activePreset;
    
    for (const TimelineEntry& entry : m_entries) {
        if (entry.type == "preset" && entry.isActive && 
            entry.positionMs <= positionMs) {
            activePreset = entry.name;
        }
    }
    
    return activePreset;
}

QString TimelineModel::activeLyricAtPosition(qint64 positionMs) const
{
    for (const TimelineEntry& entry : m_entries) {
        if (entry.type != "lyric" || !entry.isActive) continue;
        
        qint64 entryEnd = entry.positionMs + entry.durationMs;
        
        if (positionMs >= entry.positionMs && positionMs < entryEnd) {
            return entry.properties.value("text", entry.name).toString();
        }
    }
    
    return QString();
}

void TimelineModel::setDuration(qint64 durationMs)
{
    if (m_duration != durationMs) {
        m_duration = durationMs;
        emit durationChanged();
    }
}

QString TimelineModel::generateId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

int TimelineModel::findEntryIndex(const QString& id) const
{
    for (int i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].id == id) {
            return i;
        }
    }
    return -1;
}

} // namespace suno::ui