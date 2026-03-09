#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>
#include <git2.h>

namespace suno::core {

struct CommitInfo {
    QString hash;
    QString author;
    QString email;
    QString message;
    qint64 timestamp;
};

class GitController : public QObject
{
    Q_OBJECT

public:
    explicit GitController(QObject* parent = nullptr);
    ~GitController() override;
    
    bool initRepository(const QString& path);
    bool openRepository(const QString& path);
    void closeRepository();
    
    bool commit(const QString& message, const QStringList& files = {});
    bool commitAll(const QString& message);
    
    QList<CommitInfo> getHistory(int maxCount = 100);
    bool checkoutCommit(const QString& commitHash);
    
    QString currentBranch() const;
    QStringList branches() const;
    bool createBranch(const QString& name);
    bool switchBranch(const QString& name);
    
    bool hasUncommittedChanges() const;
    QStringList modifiedFiles() const;
    
    QString repositoryPath() const { return m_repoPath; }
    bool isOpen() const { return m_repo != nullptr; }

signals:
    void commitCreated(const QString& hash, const QString& message);
    void repositoryStateChanged();
    void errorOccurred(const QString& error);

private:
    bool addFiles(const QStringList& files);
    bool createCommitObject(const QString& message);
    
    git_repository* m_repo = nullptr;
    QString m_repoPath;
};

} // namespace suno::core