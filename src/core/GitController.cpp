#include "GitController.h"
#include <spdlog/spdlog.h>
#include <QDir>
#include <QFileInfo>
#include <git2/errors.h>

namespace suno::core {

GitController::GitController(QObject* parent)
    : QObject(parent)
{
    git_libgit2_init();
    spdlog::debug("libgit2 initialized");
}

GitController::~GitController()
{
    closeRepository();
    git_libgit2_shutdown();
    spdlog::debug("libgit2 shutdown");
}

bool GitController::initRepository(const QString& path)
{
    std::string pathStr = path.toStdString();
    spdlog::info("Initializing git repository at: {}", pathStr);
    
    QDir dir(path);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            spdlog::error("Failed to create repository directory");
            return false;
        }
    }
    
    git_repository* repo = nullptr;
    int error = git_repository_init(&repo, path.toUtf8().constData(), 0);
    
    if (error < 0) {
        const git_error* e = git_error_last();
        QString errorMsg = QString("Failed to init repository: %1").arg(e ? e->message : "Unknown error");
        spdlog::error("{}", errorMsg.toStdString());
        emit errorOccurred(errorMsg);
        return false;
    }
    
    if (m_repo) {
        git_repository_free(m_repo);
    }
    
    m_repo = repo;
    m_repoPath = path;
    
    spdlog::info("Repository initialized successfully");
    emit repositoryStateChanged();
    
    return true;
}

bool GitController::openRepository(const QString& path)
{
    std::string pathStr = path.toStdString();
    spdlog::info("Opening git repository at: {}", pathStr);
    
    git_repository* repo = nullptr;
    int error = git_repository_open(&repo, path.toUtf8().constData());
    
    if (error < 0) {
        const git_error* e = git_error_last();
        QString errorMsg = QString("Failed to open repository: %1").arg(e ? e->message : "Unknown error");
        spdlog::error("{}", errorMsg.toStdString());
        emit errorOccurred(errorMsg);
        return false;
    }
    
    if (m_repo) {
        git_repository_free(m_repo);
    }
    
    m_repo = repo;
    m_repoPath = path;
    
    spdlog::info("Repository opened successfully");
    emit repositoryStateChanged();
    
    return true;
}

void GitController::closeRepository()
{
    if (m_repo) {
        git_repository_free(m_repo);
        m_repo = nullptr;
        m_repoPath.clear();
        emit repositoryStateChanged();
        spdlog::debug("Repository closed");
    }
}

bool GitController::commit(const QString& message, const QStringList& files)
{
    if (!m_repo) {
        spdlog::error("No repository open");
        emit errorOccurred("No repository open");
        return false;
    }
    
    if (!addFiles(files)) {
        return false;
    }
    
    return createCommitObject(message);
}

bool GitController::commitAll(const QString& message)
{
    return commit(message, QStringList{"."});
}

bool GitController::addFiles(const QStringList& files)
{
    git_index* index = nullptr;
    int error = git_repository_index(&index, m_repo);
    
    if (error < 0) {
        const git_error* e = git_error_last();
        spdlog::error("Failed to get index: {}", e ? e->message : "Unknown");
        return false;
    }
    
    for (const QString& file : files) {
        if (file == ".") {
            error = git_index_add_all(index, nullptr, 0, nullptr, nullptr);
        } else {
            error = git_index_add_bypath(index, file.toUtf8().constData());
        }
        
        if (error < 0) {
            const git_error* e = git_error_last();
            spdlog::error("Failed to add file '{}': {}", 
                         file.toStdString(), e ? e->message : "Unknown");
            git_index_free(index);
            return false;
        }
    }
    
    error = git_index_write(index);
    git_index_free(index);
    
    if (error < 0) {
        const git_error* e = git_error_last();
        spdlog::error("Failed to write index: {}", e ? e->message : "Unknown");
        return false;
    }
    
    return true;
}

bool GitController::createCommitObject(const QString& message)
{
    git_oid tree_id, commit_id;
    git_tree* tree = nullptr;
    git_index* index = nullptr;
    git_signature* sig = nullptr;
    git_reference* ref = nullptr;
    git_commit* parent = nullptr;
    int error = 0;

    // Get index and write tree
    error = git_repository_index(&index, m_repo);
    if (error < 0) goto cleanup;

    error = git_index_write_tree(&tree_id, index);
    if (error < 0) goto cleanup;

    error = git_tree_lookup(&tree, m_repo, &tree_id);
    if (error < 0) goto cleanup;

    // Create signature
    error = git_signature_default(&sig, m_repo);
    if (error < 0) {
        error = git_signature_now(&sig, "Suno Visualizer", "user@sunovisualizer.local");
        if (error < 0) goto cleanup;
    }

    // Get parent commit (HEAD)
    error = git_reference_name_to_id(&tree_id, m_repo, "HEAD");
    if (error == 0) {
        error = git_commit_lookup(&parent, m_repo, &tree_id);
        if (error < 0 && error != GIT_ENOTFOUND) goto cleanup;
    }

    // Create commit
    {
        const git_commit* parents[] = {parent};
        error = git_commit_create_v(
            &commit_id,
            m_repo,
            "HEAD",
            sig,
            sig,
            nullptr,
            message.toUtf8().constData(),
            tree,
            parent ? 1 : 0,
            parent ? parents[0] : nullptr
        );
    }

    if (error < 0) goto cleanup;

    // Success
    {
        char oid_str[41];
        git_oid_tostr(oid_str, sizeof(oid_str), &commit_id);
        QString commitHash = QString::fromUtf8(oid_str);

        std::string hashStr = commitHash.toStdString();
        std::string msgStr = message.toStdString();
        spdlog::info("Created commit: {} - {}", hashStr, msgStr);
        emit commitCreated(commitHash, message);
    }

    cleanup:
    if (parent) git_commit_free(parent);
    if (sig) git_signature_free(sig);
    if (tree) git_tree_free(tree);
    if (index) git_index_free(index);
    if (ref) git_reference_free(ref);

    if (error < 0) {
        const git_error* e = git_error_last();
        QString errorMsg = QString("Commit failed: %1").arg(e ? e->message : "Unknown error");
        spdlog::error("{}", errorMsg.toStdString());
        emit errorOccurred(errorMsg);
        return false;
    }

    return true;
}

QList<CommitInfo> GitController::getHistory(int maxCount)
{
    QList<CommitInfo> commits;
    
    if (!m_repo) {
        return commits;
    }
    
    git_revwalk* walker = nullptr;
    git_oid oid;
    
    if (git_revwalk_new(&walker, m_repo) < 0) {
        return commits;
    }
    
    git_revwalk_sorting(walker, GIT_SORT_TIME);
    git_revwalk_push_head(walker);
    
    int count = 0;
    while (git_revwalk_next(&oid, walker) == 0 && count < maxCount) {
        git_commit* commit = nullptr;
        if (git_commit_lookup(&commit, m_repo, &oid) == 0) {
            CommitInfo info;
            
            char oid_str[41];
            git_oid_tostr(oid_str, sizeof(oid_str), &oid);
            info.hash = QString::fromUtf8(oid_str);
            
            const git_signature* author = git_commit_author(commit);
            info.author = QString::fromUtf8(author->name);
            info.email = QString::fromUtf8(author->email);
            info.timestamp = author->when.time;
            
            const char* msg = git_commit_message(commit);
            info.message = QString::fromUtf8(msg);
            
            commits.append(info);
            git_commit_free(commit);
            count++;
        }
    }
    
    git_revwalk_free(walker);
    return commits;
}

bool GitController::checkoutCommit(const QString& commitHash)
{
    std::string hashStr = commitHash.toStdString();
    spdlog::info("Checking out commit: {}", hashStr);
    return false;
}

QString GitController::currentBranch() const
{
    if (!m_repo) {
        return QString();
    }
    
    git_reference* head = nullptr;
    if (git_repository_head(&head, m_repo) < 0) {
        return QString();
    }
    
    const char* branch_name = git_reference_shorthand(head);
    QString result = QString::fromUtf8(branch_name);
    git_reference_free(head);
    
    return result;
}

QStringList GitController::branches() const
{
    // TODO: Implement branch listing
    return QStringList();
}

bool GitController::createBranch(const QString& name)
{
    std::string nameStr = name.toStdString();
    spdlog::info("Creating branch: {}", nameStr);
    return false;
}

bool GitController::switchBranch(const QString& name)
{
    std::string nameStr = name.toStdString();
    spdlog::info("Switching to branch: {}", nameStr);
    return false;
}

bool GitController::hasUncommittedChanges() const
{
    // TODO: Implement status check
    return false;
}

QStringList GitController::modifiedFiles() const
{
    // TODO: Implement modified files detection
    return QStringList();
}

} // namespace suno::core