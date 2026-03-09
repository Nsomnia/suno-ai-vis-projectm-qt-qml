#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QString>
#include <QList>

namespace suno::ui {

struct TimelineEntry {
    QString id;
    QString type;           // "preset", "lyric", "effect", "marker"
    QString name;
    QString description;
    QString gitCommitHash;
    qint64 positionMs;      // Position in timeline (milliseconds)
    qint64 durationMs;      // Duration of the entry
    QDateTime timestamp;
    bool isActive = true;
    QVariantMap properties;
};

class TimelineModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        TypeRole,
        NameRole,
        DescriptionRole,
        CommitHashRole,
        PositionRole,
        DurationRole,
        TimestampRole,
        IsActiveRole,
        PropertiesRole
    };
    Q_ENUM(Roles)

    explicit TimelineModel(QObject* parent = nullptr);
    ~TimelineModel() override = default;
    
    // QAbstractListModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    
    int count() const { return m_entries.size(); }
    qint64 duration() const { return m_duration; }
    
    // Entry management
    Q_INVOKABLE void addEntry(const QString& type, 
                               const QString& name,
                               qint64 positionMs,
                               qint64 durationMs = 0);
    
    Q_INVOKABLE void removeEntry(const QString& id);
    Q_INVOKABLE void updateEntry(const QString& id, const QVariantMap& properties);
    Q_INVOKABLE void moveEntry(const QString& id, qint64 newPositionMs);
    
    Q_INVOKABLE void clear();
    
    // Git-blame style operations
    Q_INVOKABLE void commitEntry(const QString& id, const QString& message);
    Q_INVOKABLE void revertEntry(const QString& id, const QString& commitHash);
    Q_INVOKABLE QVariantList getEntryHistory(const QString& id) const;
    
    // Playback helpers
    Q_INVOKABLE QVariantList entriesAtPosition(qint64 positionMs) const;
    Q_INVOKABLE QString activePresetAtPosition(qint64 positionMs) const;
    Q_INVOKABLE QString activeLyricAtPosition(qint64 positionMs) const;
    
    void setDuration(qint64 durationMs);

signals:
    void countChanged();
    void durationChanged();
    void entryAdded(const QString& id);
    void entryRemoved(const QString& id);
    void entryModified(const QString& id);

private:
    QString generateId() const;
    int findEntryIndex(const QString& id) const;
    
    QList<TimelineEntry> m_entries;
    qint64 m_duration = 0;
};

} // namespace suno::ui