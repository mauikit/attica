/*
    This file is part of KDE.

    Copyright (c) 2008 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2010 Sebastian Kügler <sebas@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "provider.h"

#include "accountbalance.h"
#include "accountbalanceparser.h"
#include "activity.h"
#include "activityparser.h"
#include "buildservice.h"
#include "buildserviceparser.h"
#include "buildservicejob.h"
#include "buildservicejobparser.h"
#include "privatedata.h"
#include "privatedataparser.h"
#include "category.h"
#include "categoryparser.h"
#include "comment.h"
#include "commentparser.h"
#include "content.h"
#include "contentparser.h"
#include "distribution.h"
#include "distributionparser.h"
#include "downloaditem.h"
#include "downloaditemparser.h"
#include "event.h"
#include "eventparser.h"
#include "folder.h"
#include "folderparser.h"
#include "homepagetype.h"
#include "homepagetypeparser.h"
#include "knowledgebaseentry.h"
#include "knowledgebaseentryparser.h"
#include "license.h"
#include "licenseparser.h"
#include "message.h"
#include "messageparser.h"
#include "person.h"
#include "personparser.h"
#include "platformdependent.h"
#include "postjob.h"
#include "postfiledata.h"
#include "project.h"
#include "projectparser.h"
#include "publisher.h"
#include "publisherparser.h"
#include "remoteaccount.h"
#include "remoteaccountparser.h"
#include "itemjob.h"
#include "listjob.h"

#include <QtCore/QStringList>
#include <QNetworkAccessManager>
#include <QDebug>
#include <QUrl>
#include <QNetworkReply>
#include <QtCore/QFile>


using namespace Attica;

class Provider::Private : public QSharedData
{
public:
    QUrl m_baseUrl;
    QUrl m_icon;
    QString m_name;
    QString m_credentialsUserName;
    QString m_credentialsPassword;
    QString m_personVersion;
    QString m_friendVersion;
    QString m_messageVersion;
    QString m_activityVersion;
    QString m_contentVersion;
    QString m_fanVersion;
    QString m_knowledgebaseVersion;
    QString m_eventVersion;
    QString m_commentVersion;
    PlatformDependent* m_internals;

    Private()
        :m_internals(0)
    {}

    Private(const Private& other)
        : QSharedData(other), m_baseUrl(other.m_baseUrl), m_name(other.m_name)
        , m_internals(other.m_internals), m_credentialsUserName(other.m_credentialsUserName)
        , m_credentialsPassword(other.m_credentialsPassword)
        , m_personVersion(other.m_personVersion)
        , m_friendVersion(other.m_friendVersion)
        , m_messageVersion(other.m_messageVersion)
        , m_activityVersion(other.m_activityVersion)
        , m_contentVersion(other.m_contentVersion)
        , m_fanVersion(other.m_fanVersion)
        , m_knowledgebaseVersion(other.m_knowledgebaseVersion)
        , m_eventVersion(other.m_eventVersion)
        , m_commentVersion(other.m_commentVersion)
    {
    }

Private(PlatformDependent* internals, const QUrl& baseUrl, const QString& name, const QUrl& icon,
        const QString& person, const QString& friendV, const QString& message,
        const QString& activity, const QString& content, const QString& fan,
        const QString& knowledgebase, const QString& event, const QString& comment)
        : m_baseUrl(baseUrl), m_icon(icon), m_name(name), m_internals(internals)
        , m_personVersion(person)
        , m_friendVersion(friendV)
        , m_messageVersion(message)
        , m_activityVersion(activity)
        , m_contentVersion(content)
        , m_fanVersion(fan)
        , m_knowledgebaseVersion(knowledgebase)
        , m_eventVersion(event)
        , m_commentVersion(comment)
    {
        if (m_baseUrl.isEmpty()) {
            return;
        }
        QString user;
        QString pass;
        if (m_internals->hasCredentials(m_baseUrl) && m_internals->loadCredentials(m_baseUrl, user, pass)) {
            m_credentialsUserName = user;
            m_credentialsPassword = pass;
        }
    }

    ~Private()
    {
    }
};


Provider::Provider()
    : d(new Private)
{
}

Provider::Provider(const Provider& other)
    : d(other.d)
{
}

Provider::Provider(PlatformDependent* internals, const QUrl& baseUrl, const QString& name, const QUrl& icon,
                   const QString& person, const QString& friendV, const QString& message,
                   const QString& activity, const QString& content, const QString& fan,
                   const QString& knowledgebase, const QString& event, const QString& comment)
    : d(new Private(internals, baseUrl, name, icon, person, friendV, message, activity, content, fan, knowledgebase, event, comment))
{
}

Provider& Provider::operator=(const Attica::Provider & other)
                             {
    d = other.d;
    return *this;
}

Provider::~Provider()
{
}

QUrl Provider::baseUrl() const
{
    return d->m_baseUrl;
}


bool Provider::isValid() const
{
    return d->m_baseUrl.isValid();
}

bool Provider::isEnabled() const
{
    return d->m_internals->isEnabled(d->m_baseUrl);
}

void Provider::setEnabled(bool enabled)
{
    d->m_internals->enableProvider(d->m_baseUrl, enabled);
}

QString Provider::name() const
{
    return d->m_name;
}

bool Provider::hasCredentials()
{
    return d->m_internals->hasCredentials(d->m_baseUrl);
}

bool Provider::hasCredentials() const
{
    return d->m_internals->hasCredentials(d->m_baseUrl);
}

bool Provider::loadCredentials(QString& user, QString& password)
{
    if (d->m_internals->loadCredentials(d->m_baseUrl, user, password)) {
        d->m_credentialsUserName = user;
        d->m_credentialsPassword = password;
        return true;
    }
    return false;
}

bool Provider::saveCredentials(const QString& user, const QString& password)
{
    d->m_credentialsUserName = user;
    d->m_credentialsPassword = password;
    return d->m_internals->saveCredentials(d->m_baseUrl, user, password);
}

PostJob* Provider::checkLogin(const QString& user, const QString& password)
{
    QMap<QString, QString> postParameters;

    postParameters.insert(QLatin1String( "login" ), user);
    postParameters.insert(QLatin1String( "password" ), password);

    return new PostJob(d->m_internals, createRequest(QLatin1String( "person/check" )), postParameters);
}

PostJob* Provider::registerAccount(const QString& id, const QString& password, const QString& mail, const QString& firstName, const QString& lastName)
{
    QMap<QString, QString> postParameters;

    postParameters.insert(QLatin1String( "login" ), id);
    postParameters.insert(QLatin1String( "password" ), password);
    postParameters.insert(QLatin1String( "firstname" ), firstName);
    postParameters.insert(QLatin1String( "lastname" ), lastName);
    postParameters.insert(QLatin1String( "email" ), mail);

    return new PostJob(d->m_internals, createRequest(QLatin1String( "person/add" )), postParameters);
}


ItemJob<Person>* Provider::requestPerson(const QString& id)
{
    QUrl url = createUrl( QLatin1String( "person/data/" ) + id );
    return doRequestPerson( url );
}

ItemJob<Person>* Provider::requestPersonSelf()
{
    QUrl url = createUrl( QLatin1String( "person/self" ) );
    return doRequestPerson( url );
}

ItemJob<AccountBalance>* Provider::requestAccountBalance()
{
    QUrl url = createUrl( QLatin1String( "person/balance" ) );
    return doRequestAccountBalance( url );
}

ListJob<Person>* Provider::requestPersonSearchByName(const QString& name)
{
    QUrl url = createUrl( QLatin1String( "person/data" ));
    url.addQueryItem(QLatin1String( "name" ), name);
    return doRequestPersonList( url );
}

ListJob<Person>* Provider::requestPersonSearchByLocation(qreal latitude, qreal longitude, qreal distance, int page, int pageSize)
{
    QUrl url = createUrl( QLatin1String( "person/data" ) );
    url.addQueryItem(QLatin1String( "latitude" ), QString::number(latitude));
    url.addQueryItem(QLatin1String( "longitude" ), QString::number(longitude));
    if (distance > 0.0) {
        url.addQueryItem(QLatin1String( "distance" ), QString::number(distance));
    }
    url.addQueryItem(QLatin1String( "page" ), QString::number(page));
    url.addQueryItem(QLatin1String( "pagesize" ), QString::number(pageSize));

    return doRequestPersonList( url );
}

ListJob<Person>* Provider::requestFriends(const QString& id, int page, int pageSize)
{
    QUrl url = createUrl( QLatin1String( "friend/data/" ) + id );
    url.addQueryItem(QLatin1String( "page" ), QString::number(page));
    url.addQueryItem(QLatin1String( "pagesize" ), QString::number(pageSize));

    return doRequestPersonList( url );
}

ListJob<Person>* Provider::requestSentInvitations(int page, int pageSize)
{
    QUrl url = createUrl(QLatin1String( "friend/sentinvitations" ));
    url.addQueryItem(QLatin1String( "page" ), QString::number(page));
    url.addQueryItem(QLatin1String( "pagesize" ), QString::number(pageSize));

    return doRequestPersonList(url);
}

ListJob<Person>* Provider::requestReceivedInvitations(int page, int pageSize)
{
    QUrl url = createUrl(QLatin1String( "friend/receivedinvitations" ));
    url.addQueryItem(QLatin1String( "page" ), QString::number(page));
    url.addQueryItem(QLatin1String( "pagesize" ), QString::number(pageSize));

    return doRequestPersonList(url);
}

ListJob<Activity>* Provider::requestActivities()
{
    qDebug() << "request activity";
    QUrl url = createUrl( QLatin1String( "activity" ) );
    return doRequestActivityList( url );
}


ListJob<Project>* Provider::requestProjects()
{
    qDebug() << "request projects";
    QUrl url = createUrl( "buildservice/project/list" );
    return new ListJob<Project>(d->m_internals, createRequest(url));
}

ItemJob<Project>* Provider::requestProject(const QString& id)
{
    QUrl url = createUrl( "buildservice/project/get/" + id );
    qDebug() << url;
    return new ItemJob<Project>(d->m_internals, createRequest(url));
}

QMap<QString, QString> projectPostParameters(const Project& project)
{
    QMap<QString, QString> postParameters;

    if (!project.name().isEmpty()) {
        postParameters.insert("name", project.name());
    }
    if (!project.summary().isEmpty()) {
        postParameters.insert("summary", project.summary());
    }
    if (!project.description().isEmpty()) {
        postParameters.insert("description", project.description());
    }
    if (!project.url().isEmpty()) {
        postParameters.insert("url", project.url());
    }
    if (project.developers().count()) {
        postParameters.insert("developers", project.developers().join("\n"));
    }
    if (!project.version().isEmpty()) {
        postParameters.insert("version", project.version());
    }
    if (!project.license().isEmpty()) {
        postParameters.insert("license", project.license());
    }
    if (!project.requirements().isEmpty()) {
        postParameters.insert("requirements", project.requirements());
    }
    if (!project.specFile().isEmpty()) {
        postParameters.insert("specfile", project.specFile());
    }
    return postParameters;
}

PostJob* Provider::createProject(const Project& project)
{
    return new PostJob(d->m_internals, createRequest("buildservice/project/create"),
                       projectPostParameters(project));
}

PostJob* Provider::editProject(const Project& project)
{
    return new PostJob(d->m_internals, createRequest(
                       QString("buildservice/project/edit/%1").arg(project.id())),
                       projectPostParameters(project));
}

PostJob* Provider::deleteProject(const Project& project)
{
    return new PostJob(d->m_internals, createRequest(
                       QString("buildservice/project/delete/%1").arg(project.id())),
                       projectPostParameters(project));
}

ItemJob<BuildService>* Provider::requestBuildService(const QString& id)
{
    QUrl url = createUrl( "buildservice/buildservices/get/" + id );
    return new ItemJob<BuildService>(d->m_internals, createRequest(url));
}

ItemJob<Publisher>* Provider::requestPublisher(const QString& id)
{
    qDebug() << "request publisher" << id;
    QUrl url = createUrl( "buildservice/publishing/getpublisher/" + id );
    return new ItemJob<Publisher>(d->m_internals, createRequest(url));
}

PostJob* Provider::publishBuildJob(const BuildServiceJob& buildjob, const Publisher& publisher)
{
    StringMap postParameters;
    postParameters.insert("dummyparameter", "dummyvalue");

    QString url = QString("buildservice/publishing/publishtargetresult/%1/%2").arg(
                                                                    buildjob.id(), publisher.id());
    qDebug() << "pub'ing";
    return new PostJob(d->m_internals, createRequest(url), postParameters);
}

// Buildservices and their jobs
ItemJob<BuildServiceJob>* Provider::requestBuildServiceJob(const QString &id)
{
    QUrl url = createUrl( "buildservice/jobs/get/" + id );
    qDebug() << url;
    return new ItemJob<BuildServiceJob>(d->m_internals, createRequest(url));
}

QMap<QString, QString> buildServiceJobPostParameters(const BuildServiceJob& buildjob)
{
    QMap<QString, QString> postParameters;

    if (!buildjob.name().isEmpty()) {
        postParameters.insert("name", buildjob.name());
    }
    if (!buildjob.projectId().isEmpty()) {
        postParameters.insert("projectid", buildjob.projectId());
    }
    if (!buildjob.target().isEmpty()) {
        postParameters.insert("target", buildjob.target());
    }
    if (!buildjob.buildServiceId().isEmpty()) {
        postParameters.insert("buildservice", buildjob.buildServiceId());
    }
}

PostJob* Provider::cancelBuildServiceJob(const BuildServiceJob& job)
{
    StringMap postParameters;
    postParameters.insert("dummyparameter", "dummyvalue");
    qDebug() << "b....................b";
    return new PostJob(d->m_internals, createRequest(
                       QString("buildservice/jobs/cancel/%1").arg(job.id())), postParameters);
}

PostJob* Provider::createBuildServiceJob(const BuildServiceJob& job)
{
    StringMap postParameters;
    // A postjob won't be run without parameters. 
    // so even while we don't need any in this case,
    // we add dummy data to the request
    postParameters.insert("dummyparameter", "dummyvalue");
    qDebug() << "Creating new BSJ on" << job.buildServiceId();
    return new PostJob(d->m_internals, createRequest(
                       QString("buildservice/jobs/create/%1/%2/%3").arg(
                           job.projectId(), job.buildServiceId(), job.target())),
                       postParameters);
}

ListJob<BuildService>* Provider::requestBuildServices()
{
    qDebug() << "request projects";
    QUrl url = createUrl( "buildservice/buildservices/list" );
    return new ListJob<BuildService>(d->m_internals, createRequest(url));
}

ListJob<Publisher>* Provider::requestPublishers()
{
    QUrl url = createUrl( "buildservice/publishing/getpublishingcapabilities" );
    qDebug() << "request publishers" << url;
    return new ListJob<Publisher>(d->m_internals, createRequest(url));
}

ListJob<BuildServiceJob>* Provider::requestBuildServiceJobs(const Project &project)
{
    qDebug() << "request projects";
    QUrl url = createUrl( "buildservice/jobs/list/" + project.id() );
    return new ListJob<BuildServiceJob>(d->m_internals, createRequest(url));
}

ListJob<RemoteAccount>* Provider::requestRemoteAccounts()
{
    qDebug() << "request remoteaccounts";
    QUrl url = createUrl( "buildservice/remoteaccounts/list/");
    return new ListJob<RemoteAccount>(d->m_internals, createRequest(url));
}

PostJob* Provider::createRemoteAccount(const RemoteAccount& account)
{
    StringMap postParameters;
    // A postjob won't be run without parameters.
    // so even while we don't need any in this case,
    // we add dummy data to the request
    postParameters.insert("login", account.login());
    postParameters.insert("password", account.password());
    postParameters.insert("type", account.type());
    postParameters.insert("typeid", account.remoteServiceId()); // FIXME: remoteserviceid?
    postParameters.insert("data", account.data());
    qDebug() << "Creating new Remoteaccount" << account.id() << account.login() << account.password();
    return new PostJob(d->m_internals, createRequest("buildservice/remoteaccounts/add"),
                       postParameters);
}

PostJob* Provider::editRemoteAccount(const RemoteAccount& account)
{
    StringMap postParameters;
    // A postjob won't be run without parameters.
    // so even while we don't need any in this case,
    // we add dummy data to the request
    postParameters.insert("login", account.login());
    postParameters.insert("password", account.password());
    postParameters.insert("type", account.type());
    postParameters.insert("typeid", account.remoteServiceId()); // FIXME: remoteserviceid?
    postParameters.insert("data", account.data());
    qDebug() << "Creating new Remoteaccount" << account.id() << account.login() << account.password();
    return new PostJob(d->m_internals, createRequest("buildservice/remoteaccounts/edit/" + account.id()),
                       postParameters);
}

ItemJob<RemoteAccount>* Provider::requestRemoteAccount(const QString &id)
{
    QUrl url = createUrl( "buildservice/remoteaccounts/get/" + id );
    qDebug() << url;
    return new ItemJob<RemoteAccount>(d->m_internals, createRequest(url));
}

PostJob* Provider::deleteRemoteAccount(const QString& id)
{
    StringMap postParameters;
    return new PostJob(d->m_internals, createRequest(
                       QString("buildservice/remoteaccounts/remove/%1").arg(id)),
                       postParameters);
}

PostJob* Provider::postActivity(const QString& message)
{
    StringMap postParameters;
    postParameters.insert(QLatin1String( "message" ), message);
    return new PostJob(d->m_internals, createRequest(QLatin1String( "activity" )), postParameters);
}

PostJob* Provider::inviteFriend(const QString& to, const QString& message)
{
    StringMap postParameters;
    postParameters.insert(QLatin1String( "message" ), message);
    return new PostJob(d->m_internals, createRequest(QLatin1String( "friend/invite/" ) + to), postParameters);
}


PostJob* Provider::approveFriendship(const QString& to)
{
    return new PostJob(d->m_internals, createRequest(QLatin1String( "friend/approve/" ) + to));
}


PostJob* Provider::declineFriendship(const QString& to)
{
    return new PostJob(d->m_internals, createRequest(QLatin1String( "friend/decline/" ) + to));
}

PostJob* Provider::cancelFriendship(const QString& to)
{
    return new PostJob(d->m_internals, createRequest(QLatin1String( "friend/cancel/" ) + to));
}


PostJob* Provider::postLocation(qreal latitude, qreal longitude, const QString& city, const QString& country)
{
    StringMap postParameters;
    postParameters.insert(QLatin1String( "latitude" ), QString::number(latitude));
    postParameters.insert(QLatin1String( "longitude" ), QString::number(longitude));
    postParameters.insert(QLatin1String( "city" ), city);
    postParameters.insert(QLatin1String( "country" ), country);
    return new PostJob(d->m_internals, createRequest(QLatin1String( "person/self" )), postParameters);
}


ListJob<Folder>* Provider::requestFolders()
{
    return doRequestFolderList( createUrl( QLatin1String( "message" ) ) );
}

ListJob<Message>* Provider::requestMessages(const Folder& folder)
{
    return doRequestMessageList( createUrl( QLatin1String( "message/" ) + folder.id() ) );
}


ListJob<Message>* Provider::requestMessages(const Folder& folder, Message::Status status)
{
    QUrl url = createUrl(QLatin1String( "message/" ) + folder.id());
    url.addQueryItem(QLatin1String( "status" ), QString::number(status));
    return doRequestMessageList(url);
}


ItemJob<Message>* Provider::requestMessage(const Folder& folder, const QString& id)
{
    return new ItemJob<Message>(d->m_internals, createRequest(QLatin1String( "message/" ) + folder.id() + QLatin1Char( '/' ) + id));
}


PostJob* Provider::postMessage( const Message &message )
{
    StringMap postParameters;
    postParameters.insert(QLatin1String( "message" ), message.body());
    postParameters.insert(QLatin1String( "subject" ), message.subject());
    postParameters.insert(QLatin1String( "to" ), message.to());
    return new PostJob(d->m_internals, createRequest(QLatin1String( "message/2" )), postParameters);
}

ListJob<Category>* Provider::requestCategories()
{
    QUrl url = createUrl( QLatin1String( "content/categories" ) );
    ListJob<Category> *job = new ListJob<Category>(d->m_internals, createRequest(url));
    return job;
}

ListJob< License >* Provider::requestLicenses()
{
    QUrl url = createUrl( QLatin1String( "content/licenses" ) );
    ListJob<License> *job = new ListJob<License>(d->m_internals, createRequest(url));
    return job;
}

ListJob< Distribution >* Provider::requestDistributions()
{
    QUrl url = createUrl( QLatin1String( "content/distributions" ) );
    ListJob<Distribution> *job = new ListJob<Distribution>(d->m_internals, createRequest(url));
    return job;
}

ListJob< HomePageType >* Provider::requestHomePageTypes()
{
    QUrl url = createUrl( QLatin1String( "content/homepages" ) );
    ListJob<HomePageType> *job = new ListJob<HomePageType>(d->m_internals, createRequest(url));
    return job;
}

ListJob<Content>* Provider::searchContents(const Category::List& categories, const QString& search, SortMode sortMode, uint page, uint pageSize)
{
    return searchContents(categories, QString(), Distribution::List(), License::List(), search, sortMode, page, pageSize);
}

ListJob<Content>* Provider::searchContentsByPerson(const Category::List& categories, const QString& person, const QString& search, SortMode sortMode, uint page, uint pageSize)
{
    return searchContents(categories, person, Distribution::List(), License::List(), search, sortMode, page, pageSize);
}

ListJob<Content>* Provider::searchContents(const Category::List& categories, const QString& person, const Distribution::List& distributions, const License::List& licenses, const QString& search, SortMode sortMode, uint page, uint pageSize)
{
    QUrl url = createUrl( QLatin1String( "content/data" ) );

    QStringList categoryIds;
    foreach( const Category &category, categories ) {
        categoryIds.append( category.id() );
    }
    url.addQueryItem( QLatin1String( "categories" ), categoryIds.join( QLatin1String( "x" ) ) );

    QStringList distributionIds;
    foreach( const Distribution &distribution, distributions) {
        distributionIds.append( QString(distribution.id()) );
    }
    url.addQueryItem( QLatin1String( "distribution" ), distributionIds.join( QLatin1String( "," ) ) );

    QStringList licenseIds;
    foreach( const License &license, licenses) {
        licenseIds.append( QString(license.id()) );
    }
    url.addQueryItem( QLatin1String( "license" ), licenseIds.join( QLatin1String( "," ) ) );

    if (!person.isEmpty()) {
        url.addQueryItem( QLatin1String( "user" ), person );
    }

    url.addQueryItem( QLatin1String( "search" ), search );
    QString sortModeString;
    switch ( sortMode ) {
    case Newest:
        sortModeString = QLatin1String( "new" );
        break;
    case Alphabetical:
        sortModeString = QLatin1String( "alpha" );
        break;
    case Rating:
        sortModeString = QLatin1String( "high" );
        break;
    case Downloads:
        sortModeString = QLatin1String( "down" );
        break;
    }

    if ( !sortModeString.isEmpty() ) {
        url.addQueryItem( QLatin1String( "sortmode" ), sortModeString );
    }

    url.addQueryItem( QLatin1String( "page" ), QString::number(page) );
    url.addQueryItem( QLatin1String( "pagesize" ), QString::number(pageSize) );

    ListJob<Content> *job = new ListJob<Content>(d->m_internals, createRequest(url));
    return job;
}

ItemJob<Content>* Provider::requestContent(const QString& id)
{
    QUrl url = createUrl( QLatin1String( "content/data/" ) + id );
    ItemJob<Content> *job = new ItemJob<Content>(d->m_internals, createRequest(url));
    return job;
}

ItemPostJob<Content>* Provider::addNewContent(const Category& category, const Content& cont)
{
    if (!category.isValid()) {
        return 0;
    }

    QUrl url = createUrl(QLatin1String( "content/add" ));
    StringMap pars(cont.attributes());

    pars.insert(QLatin1String( "type" ), category.id());
    pars.insert(QLatin1String( "name" ), cont.name());

    qDebug() << "Parameter map: " << pars;

    return new ItemPostJob<Content>(d->m_internals, createRequest(url), pars);
}


ItemPostJob<Content>* Provider::editContent(const Category& updatedCategory, const QString& contentId, const Content& updatedContent)
{
    // FIXME I get a server error message here, though the name of the item is changed
    QUrl url = createUrl(QLatin1String( "content/edit/" ) + contentId);
    StringMap pars(updatedContent.attributes());

    pars.insert(QLatin1String( "type" ), updatedCategory.id());
    pars.insert(QLatin1String( "name" ), updatedContent.name());

    qDebug() << "Parameter map: " << pars;

    return new ItemPostJob<Content>(d->m_internals, createRequest(url), pars);
}

/*
PostJob* Provider::setDownloadFile(const QString& contentId, QIODevice* payload)
{
    QUrl url = createUrl("content/uploaddownload/" + contentId);
    PostFileData postRequest(url);
    // FIXME mime type
    //postRequest.addFile("localfile", payload, "application/octet-stream");
    postRequest.addFile("localfile", payload, "image/jpeg");
    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}
*/

PostJob* Provider::deleteContent(const QString& contentId)
{
    QUrl url = createUrl(QLatin1String( "content/delete/" ) + contentId);
    PostFileData postRequest(url);
    postRequest.addArgument(QLatin1String( "contentid" ), contentId);
    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}

PostJob* Provider::setDownloadFile(const QString& contentId, const QString& fileName, const QByteArray& payload)
{
    QUrl url = createUrl(QLatin1String( "content/uploaddownload/" ) + contentId);
    PostFileData postRequest(url);
    // FIXME mime type
    postRequest.addFile(fileName, payload, QLatin1String( "application/octet-stream" ));
    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}

PostJob* Provider::deleteDownloadFile(const QString& contentId)
{
    QUrl url = createUrl(QLatin1String( "content/deletedownload/" ) + contentId);
    PostFileData postRequest(url);
    postRequest.addArgument(QLatin1String( "contentid" ), contentId);
    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}

PostJob* Provider::setPreviewImage(const QString& contentId, const QString& previewId, const QString& fileName, const QByteArray& image)
{
    QUrl url = createUrl(QLatin1String( "content/uploadpreview/" ) + contentId + QLatin1Char( '/' ) + previewId);

    PostFileData postRequest(url);
    postRequest.addArgument(QLatin1String( "contentid" ), contentId);
    postRequest.addArgument(QLatin1String( "previewid" ), previewId);
    // FIXME mime type
    postRequest.addFile(fileName, image, QLatin1String( "application/octet-stream" ));

    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}

PostJob* Provider::deletePreviewImage(const QString& contentId, const QString& previewId)
{
    QUrl url = createUrl(QLatin1String( "content/deletepreview/" ) + contentId + QLatin1Char( '/' ) + previewId);
    PostFileData postRequest(url);
    postRequest.addArgument(QLatin1String( "contentid" ), contentId);
    postRequest.addArgument(QLatin1String( "previewid" ), previewId);
    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}

PostJob* Provider::voteForContent(const QString& contentId, bool positiveVote)
{
    StringMap postParameters;
    postParameters.insert(QLatin1String( "vote" ), positiveVote ? QLatin1String( "good" ) : QLatin1String( "bad" ));
    qDebug() << "vote: " << positiveVote;
    return new PostJob(d->m_internals, createRequest(QLatin1String( "content/vote/" ) + contentId), postParameters);
}

PostJob* Provider::voteForContent(const QString& contentId, uint rating)
{
    // according to OCS API, the rating is 0..100
    Q_ASSERT(rating <= 100);
    StringMap postParameters;
    postParameters.insert(QLatin1String( "vote" ), QString::number(rating));
    qDebug() << "vote: " << QString::number(rating);
    return new PostJob(d->m_internals, createRequest(QLatin1String( "content/vote/" ) + contentId), postParameters);
}

PostJob* Provider::becomeFan(const QString& contentId)
{
    QUrl url = createUrl(QLatin1String( "fan/add/" ) + contentId);
    PostFileData postRequest(url);
    postRequest.addArgument(QLatin1String( "contentid" ), contentId);
    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}

ListJob<Person>* Provider::requestFans(const QString& contentId, uint page, uint pageSize)
{
    QUrl url = createUrl( QLatin1String( "fan/data/" ) + contentId );
    url.addQueryItem( QLatin1String( "contentid" ), contentId );
    url.addQueryItem( QLatin1String( "page" ), QString::number(page) );
    url.addQueryItem( QLatin1String( "pagesize" ), QString::number(pageSize) );
    ListJob<Person> *job = new ListJob<Person>(d->m_internals, createRequest(url));
    return job;
}

ItemJob<DownloadItem>* Provider::downloadLink(const QString& contentId, const QString& itemId)
{
    QUrl url = createUrl( QLatin1String( "content/download/" ) + contentId + QLatin1Char( '/' ) + itemId );
    ItemJob<DownloadItem> *job = new ItemJob<DownloadItem>(d->m_internals, createRequest(url));
    return job;
}

ItemJob<KnowledgeBaseEntry>* Provider::requestKnowledgeBaseEntry(const QString& id)
{
    QUrl url = createUrl( QLatin1String( "knowledgebase/data/" ) + id );
    ItemJob<KnowledgeBaseEntry> *job = new ItemJob<KnowledgeBaseEntry>(d->m_internals, createRequest(url));
    return job;
}

ListJob<KnowledgeBaseEntry>* Provider::searchKnowledgeBase(const Content& content, const QString& search, Provider::SortMode sortMode, int page, int pageSize)
{
    QUrl url = createUrl( QLatin1String( "knowledgebase/data" ) );
    if (content.isValid()) {
        url.addQueryItem(QLatin1String( "content" ), content.id());
    }

    url.addQueryItem( QLatin1String( "search" ), search );
    QString sortModeString;
    switch ( sortMode ) {
    case Newest:
        sortModeString = QLatin1String( "new" );
        break;
    case Alphabetical:
        sortModeString = QLatin1String( "alpha" );
        break;
    case Rating:
        sortModeString = QLatin1String( "high" );
        break;
        //FIXME: knowledge base doesn't have downloads
    case Downloads:
        sortModeString = QLatin1String( "new" );
        break;
    }
    if ( !sortModeString.isEmpty() ) {
        url.addQueryItem( QLatin1String( "sortmode" ), sortModeString );
    }

    url.addQueryItem( QLatin1String( "page" ), QString::number(page) );
    url.addQueryItem( QLatin1String( "pagesize" ), QString::number(pageSize) );

    ListJob<KnowledgeBaseEntry> *job = new ListJob<KnowledgeBaseEntry>(d->m_internals, createRequest(url));
    return job;
}

ItemJob<Event>* Provider::requestEvent(const QString& id)
{
    ItemJob<Event>* job = new ItemJob<Event>(d->m_internals, createRequest(QLatin1String( "event/data/" ) + id));
    return job;
}

ListJob<Event>* Provider::requestEvent(const QString& country, const QString& search, const QDate& startAt, Provider::SortMode mode, int page, int pageSize)
{
    QUrl url = createUrl(QLatin1String( "event/data" ));

    if (!search.isEmpty()) {
        url.addQueryItem(QLatin1String( "search" ), search);
    }

    QString sortModeString;
    switch (mode) {
    case Newest:
        sortModeString = QLatin1String( "new" );
        break;
    case Alphabetical:
        sortModeString = QLatin1String( "alpha" );
        break;
    default:
        break;
    }
    if (!sortModeString.isEmpty()) {
        url.addQueryItem(QLatin1String( "sortmode" ), sortModeString);
    }

    if (!country.isEmpty()) {
        url.addQueryItem(QLatin1String( "country" ), country);
    }

    url.addQueryItem(QLatin1String( "startat" ), startAt.toString(Qt::ISODate));

    url.addQueryItem(QLatin1String( "page" ), QString::number(page));
    url.addQueryItem(QLatin1String( "pagesize" ), QString::number(pageSize));

    ListJob<Event>* job = new ListJob<Event>(d->m_internals, createRequest(url));
    return job;
}

ListJob<Comment>* Provider::requestComments(const Comment::Type commentType, const QString& id, const QString& id2, int page, int pageSize)
{
    QString commentTypeString;
    commentTypeString = Comment::commentTypeToString(commentType);
    if (commentTypeString.isEmpty()) {
        return 0;
    }

    QUrl url = createUrl(QLatin1String( "comments/data/" ) + commentTypeString + QLatin1String( "/" ) + id + QLatin1String( "/" ) + id2);

    url.addQueryItem(QLatin1String( "page" ), QString::number(page));
    url.addQueryItem(QLatin1String( "pagesize" ), QString::number(pageSize));

    ListJob<Comment>* job = new ListJob<Comment>(d->m_internals, createRequest(url));
    return job;
}

ItemPostJob<Comment>* Provider::addNewComment(const Comment::Type commentType, const QString& id, const QString& id2, const QString& parentId, const QString &subject, const QString& message)
{
    QString commentTypeString;
    commentTypeString = Comment::commentTypeToString(commentType);
    if (commentTypeString.isEmpty()) {
        return 0;
    }

    QMap<QString, QString> postParameters;

    postParameters.insert(QLatin1String( "type" ), commentTypeString);
    postParameters.insert(QLatin1String( "content" ), id);
    postParameters.insert(QLatin1String( "content2" ), id2);
    postParameters.insert(QLatin1String( "parent" ), parentId);
    postParameters.insert(QLatin1String( "subject" ), subject);
    postParameters.insert(QLatin1String( "message" ), message);

    return new ItemPostJob<Comment>(d->m_internals, createRequest(QLatin1String( "comments/add" )), postParameters);
}

PostJob* Provider::voteForComment(const QString & id, uint rating)
{
    if (rating > 100) {
        return 0;
    }

    QMap<QString, QString> postParameters;
    postParameters.insert(QLatin1String( "vote" ), QString::number(rating));

    QUrl url = createUrl(QLatin1String( "comments/vote/" ) + id);
    return new PostJob(d->m_internals, createRequest(url), postParameters);
}

PostJob* Provider::setPrivateData(const QString& app, const QString& key, const QString& value)
{
    QUrl url = createUrl(QLatin1String( "privatedata/setattribute/" ) + app + QLatin1String( "/" ) + key);
    PostFileData postRequest(url);

    postRequest.addArgument(QLatin1String( "value" ), value);

    return new PostJob(d->m_internals, postRequest.request(), postRequest.data());
}

ItemJob<PrivateData>* Provider::requestPrivateData(const QString& app, const QString& key)
{
    ItemJob<PrivateData>* job = new ItemJob<PrivateData>(d->m_internals, createRequest(QLatin1String( "privatedata/getattribute/" ) + app + QLatin1String( "/" ) + key));
    return job;
}

QUrl Provider::createUrl(const QString& path)
{
    QUrl url(d->m_baseUrl.toString() + path);
    if (!d->m_credentialsUserName.isEmpty()) {
        url.setUserName(d->m_credentialsUserName);
        url.setPassword(d->m_credentialsPassword);
    }
    return url;
}

QNetworkRequest Provider::createRequest(const QUrl& url)
{
    QNetworkRequest request(url);
    qDebug() << "OCS Request:" << url;
    if (!d->m_credentialsUserName.isEmpty()) {
        QString concatenated = d->m_credentialsUserName + QLatin1Char( ':' ) + d->m_credentialsPassword;
        QByteArray data = concatenated.toLocal8Bit().toBase64();
        QString headerData = QLatin1String( "Basic " ) +QLatin1String( data );
        request.setRawHeader("Authorization" ,headerData.toLocal8Bit() );
    }

    return request;
}

QNetworkRequest Provider::createRequest(const QString& path)
{
    return createRequest(createUrl(path));
}

ItemJob<Person>* Provider::doRequestPerson(const QUrl& url)
{
    return new ItemJob<Person>(d->m_internals, createRequest(url));
}

ItemJob<AccountBalance>* Provider::doRequestAccountBalance(const QUrl& url)
{
    return new ItemJob<AccountBalance>(d->m_internals, createRequest(url));
}

ListJob<Person>* Provider::doRequestPersonList(const QUrl& url)
{
    return new ListJob<Person>(d->m_internals, createRequest(url));
}

ListJob<Activity>* Provider::doRequestActivityList(const QUrl& url)
{
    return new ListJob<Activity>(d->m_internals, createRequest(url));
}

ListJob<Folder>* Provider::doRequestFolderList(const QUrl& url)
{
    return new ListJob<Folder>(d->m_internals, createRequest(url));
}

ListJob<Message>* Provider::doRequestMessageList(const QUrl& url)
{
    return new ListJob<Message>(d->m_internals, createRequest(url));
}

QString Provider::activityServiceVersion() const
{
    return d->m_activityVersion;
}
QString Provider::commentServiceVersion() const
{
    return d->m_commentVersion;
}
QString Provider::contentServiceVersion() const
{
    return d->m_contentVersion;
}
QString Provider::fanServiceVersion() const
{
    return d->m_fanVersion;
}
QString Provider::friendServiceVersion() const
{
    return d->m_friendVersion;
}
QString Provider::knowledgebaseServiceVersion() const
{
    return d->m_knowledgebaseVersion;
}
QString Provider::messageServiceVersion() const
{
    return d->m_messageVersion;
}
QString Provider::personServiceVersion() const
{
    return d->m_personVersion;
}

bool Provider::hasActivityService() const
{
    return !d->m_activityVersion.isEmpty();
}
bool Provider::hasCommentService() const
{
    return !d->m_commentVersion.isEmpty();
}
bool Provider::hasContentService() const
{
    return !d->m_contentVersion.isEmpty();
}
bool Provider::hasFanService() const
{
    return !d->m_fanVersion.isEmpty();
}
bool Provider::hasFriendService() const
{
    return !d->m_friendVersion.isEmpty();
}
bool Provider::hasKnowledgebaseService() const
{
    return !d->m_knowledgebaseVersion.isEmpty();
}
bool Provider::hasMessageService() const
{
    return !d->m_messageVersion.isEmpty();
}
bool Provider::hasPersonService() const
{
    return !d->m_personVersion.isEmpty();
}
