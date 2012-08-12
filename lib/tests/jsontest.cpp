/*
    This file is part of KDE.

    Copyright (c) 2012 Felix Rohrbach <fxrh@gmx>

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

#include "jsonparser.h"
#include "metadata.h"
#include "achievement.h"
#include "activity.h"
#include "comment.h"
#include "content.h"
#include "distribution.h"
#include "downloaditem.h"
#include "homepagetype.h"
#include "icon.h"
#include "license.h"
#include "message.h"
#include "person.h"

#include <QtTest>
#include <QObject>
#include <QtCore/QDateTime>

using namespace Attica;

class JsonTest: public QObject
{
    Q_OBJECT
private slots:
    void testMetadata();

    void testAchievement();
    void testActivity();
    void testCategory();
    void testComment();
    void testContent();
    void testDistribution();
    void testDownloadItem();
    void testHomepageType();
    void testIcon();
    void testLicense();
    void testMessage();
    void testPerson();

private:
    static const QString startString;
    static const QString endString;
};

const QString JsonTest::startString = QLatin1String("{"
            "\"status\": \"ok\","
            "\"statuscode\": 100,"
            "\"message\": null,"
            "\"data\": ");

const QString JsonTest::endString = QLatin1String("}");

void JsonTest::testMetadata()
{
    QString testData1 = QLatin1String("{"
            "\"status\":\"ok\","
            "\"statuscode\":100,"
            "\"message\":null"
            "}");
    JsonParser<NoneType> parser;
    parser.parse(testData1);
    Metadata metadata = parser.metadata();
    QCOMPARE( metadata.statusString(), QLatin1String("ok") );
    QCOMPARE( metadata.statusCode(), 100 );
    QCOMPARE( metadata.message(), QLatin1String("") );
}

void JsonTest::testAchievement()
{
    QString testData1 = startString+QLatin1String("["
        "{"
            "\"id\": 12,"
            "\"content_id\": 1234,"
            "\"name\": \"The Great Thing\","
            "\"description\": \"Great things are afoot\","
            "\"explanation\": \"A great thing which will help you with other things.\","
            "\"points\": 15,"
            "\"image\": \"https://opendesktop.org/content/achievements/images/12.png\","
            "\"dependencies\": null,"
            "\"visibility\": \"visible\","
            "\"type\": \"flowing\","
            "\"progress\": 1"
        "},"
        "{"
            "\"id\": 321,"
            "\"content_id\": 1234,"
            "\"name\": \"The Greater Thing\","
            "\"description\": \"Even greater things are afoot\","
            "\"explanation\": \"Incredible things have been achieved.\","
            "\"points\": 25,"
            "\"image\": \"https://opendesktop.org/content/achievements/images/12.png\","

            "\"dependencies\": ["
                "{"
                    "\"achievement_id\": 12"
                "}"
            "],"

            "\"visibility\": \"dependents\","
            "\"type\": \"set\","

            "\"options\": ["
                "{"
                    "\"option\": \"good\""
                "},"
                "{"
                    "\"option\": \"other good\""
                "},"
                "{"
                    "\"option\": \"also good\""
                "}"
            "],"

            "\"progress\": ["
                "{"
                    "\"reached\": \"good\""
                "},"
                "{"
                    "\"reached\": \"also good\""
                "}"
            "]"
        "}"
        "]") + endString;
    JsonParser<Achievement> parser;
    parser.parse( testData1 );
    Achievement achievement1 = parser.itemList().at(0);
    Achievement achievement2 = parser.itemList().at(1);

    QVERIFY( achievement1.isValid() );
    QVERIFY( achievement2.isValid() );

    QCOMPARE( achievement1.id(), QLatin1String("12") );
    QCOMPARE( achievement1.contentId(), QLatin1String("1234") );
    QCOMPARE( achievement1.name(), QLatin1String("The Great Thing") );
    QCOMPARE( achievement1.description(), QLatin1String("Great things are afoot") );
    QCOMPARE( achievement1.explanation(), QLatin1String("A great thing which will help you with other things.") );
    QCOMPARE( achievement1.points(), 15 );
    QCOMPARE( achievement1.image(), QUrl(QLatin1String("https://opendesktop.org/content/achievements/images/12.png")) );
    QVERIFY( achievement1.dependencies().isEmpty() );
    QCOMPARE( achievement1.visibility(), Achievement::VisibleAchievement );
    QCOMPARE( achievement1.type(), Achievement::FlowingAchievement );
    QCOMPARE( achievement1.progress(), QVariant::fromValue<int>(1) );

    QCOMPARE( achievement2.id(), QLatin1String("321") );
    QCOMPARE( achievement2.dependencies().size(), 1 );
    QCOMPARE( achievement2.dependencies().at(0), QLatin1String("12") );
    QCOMPARE( achievement2.options().size(), 3 );
    QCOMPARE( achievement2.options().at(2), QLatin1String("also good") );
    QVERIFY( achievement2.progress().toStringList().contains(QLatin1String("also good")) );
}

void JsonTest::testActivity()
{
    QString testData = startString + QLatin1String("["
        "{"
            "\"details\": \"full\","
            "\"id\": 42,"
            "\"personid\": \"lpapp\","
            "\"firstname\": \"Laszlo\","
            "\"lastname\": \"Papp\","
            "\"profilepage\": \"/usermanager/search.php?username=lpapp\","
            "\"avatarpic\": \"https://opendesktop.org/usermanager/nopic.png\","
            "\"timestamp\": \"2008-08-01T20:30:19+02:00\","
            "\"type\": 6,"
            "\"message\": \"testy2 has updated: &quot;Extract And Compress&quot;\","
            "\"link\": \"https://www.KDE-Look.org/content/show.php?content=84206\""
        "},"
        "{"
            "\"details\": \"full\","
            "\"id\": 43,"
            "\"personid\": \"foobar\","
            "\"firstname\": \"Foo\","
            "\"lastname\": \"Bar\","
            "\"profilpage\": \"/usermanager/search.php?username=foobar\","
            "\"avatarpic\": \"https://www.opendesktop.org/usermanager/nopic.png\","
            "\"timestamp:\": \"2008-08-02T19:38:10+02:00\","
            "\"type\": 6,"
            "\"message\": \"foobar2 has updated: &quot;Arezzo&quot;\","
            "\"link\": \"https://www.KDE-Look.org/content/show.php?content=84403\""
        "}"
        "]") + endString;
    JsonParser<Activity> parser;
    parser.parse( testData );
    Activity activity = parser.item();

    QVERIFY( activity.isValid() );
    QCOMPARE( activity.id(), QLatin1String("42") );
    QVERIFY( activity.associatedPerson().isValid() );
    QCOMPARE( activity.associatedPerson().id(), QLatin1String("lpapp") );
    QCOMPARE( activity.associatedPerson().firstName(), QLatin1String("Laszlo") );
    QCOMPARE( activity.associatedPerson().lastName(), QLatin1String("Papp") );
    QCOMPARE( activity.associatedPerson().avatarUrl(), QUrl(QLatin1String("https://opendesktop.org/usermanager/nopic.png")) );
    QCOMPARE( activity.timestamp(), QDateTime::fromString(QLatin1String("2008-08-01T20:30:19+02:00"), Qt::ISODate) );
    QCOMPARE( activity.message(), QLatin1String("testy2 has updated: &quot;Extract And Compress&quot;") );
    QCOMPARE( activity.link(), QUrl(QLatin1String("https://www.KDE-Look.org/content/show.php?content=84206")) );
}

void JsonTest::testCategory()
{
    QString testData = startString + QLatin1String("["
        "{"
            "\"id\": 1,"
            "\"name\": \"KDE Wallpaper 640x480\""
        "},"
        "{"
            "\"id\": 2,"
            "\"name\": \"KDE Wallpaper 800x600\""
        "},"
        "{"
            "\"id\": 3,"
            "\"name\": \"KDE Wallpaper 1024x768\""
        "},"
        "{"
            "\"id\": 4,"
            "\"name\": \"KDE Wallpaper 1280x1024\""
        "}"
        "]") + endString;
    JsonParser<Category> parser;
    parser.parse( testData );
    QCOMPARE( parser.itemList().size(), 4 );
    Category category = parser.itemList().at(0);

    QVERIFY( category.isValid() );
    QCOMPARE( category.id(), QLatin1String("1") );
    QCOMPARE( category.name(), QLatin1String("KDE Wallpaper 640x480") );
}

void JsonTest::testComment()
{
    QString testData = startString + QLatin1String("["
        "{"
            "\"id\": 235,"
            "\"subject\": \"vxvdfvd\","
            "\"text\": \"gfdgfdgfgfgf\","
            "\"childcount\": 1,"
            "\"user\": \"test\","
            "\"date\": \"2005-01-29T19:17:06+01:00\","
            "\"score\": 60,"

            "\"children\": ["
                "{"
                    "\"id\": 315,"
                    "\"subject\": \"Re: jjjjjjjjjjjjjjj\","
                    "\"text\": \"gfdg\","
                    "\"childcount\": 0,"
                    "\"user\": \"lpapp\","
                    "\"date\": \"2007-03-13T21:34:43+01:00\","
                    "\"score\": 40"
                "}"
            "]"
        "}"
        "]") + endString;
    JsonParser<Comment> parser;
    parser.parse( testData );
    Comment comment = parser.item();

    QVERIFY( comment.isValid() );
    QCOMPARE( comment.id(), QLatin1String("235") );
    QCOMPARE( comment.subject(), QLatin1String("vxvdfvd") );
    QCOMPARE( comment.text(), QLatin1String("gfdgfdgfgfgf") );
    QCOMPARE( comment.childCount(), 1 );
    QCOMPARE( comment.user(), QLatin1String("test") );
    QCOMPARE( comment.date(), QDateTime::fromString(QLatin1String("2005-01-29T19:17:06+01:00"), Qt::ISODate) );
    QCOMPARE( comment.score(), 60 );
    QCOMPARE( comment.children().at(0).id(), QLatin1String("315") );
}

void JsonTest::testContent()
{
    QString testData = startString + QLatin1String("["
        "{"
            "\"details\": \"full\","
            "\"id\": 100,"
            "\"name\": \"GradE8\","
            "\"version\": null,"
            "\"summary\": \"this is a short summary\","
            "\"changed\": \"2001-09-28T18:45:40+02:00\","
            "\"created\": \"2001-09-28T18:45:40+02:00\","
            "\"typeid\": 10,"
            "\"typename\": \"Theme/Style for KDE 2.1\","
            "\"language\": null,"
            "\"personid\": \"Hans\","
            "\"downloads\": 2,"
            "\"score\": 67,"
            "\"comments\": 0,"
            "\"commentspage\": \"https://www.opendesktop.org/content/show.php?content=100\","
            "\"fans\": 22,"
            "\"fanspage\": \"https://www.opendesktop.org/content/show.php?action=fan&amp;content=100\","
            "\"knowledgebasentries\": 7,"
            "\"knowledgebasepage\": \"https://www.opendesktop.org/content/show.php?action=knowledgebase&amp;content=100\","
            "\"depend\": null,"
            "\"preview1\": \"https://www.KDE-Look.org/content/preview.php?preview=1&amp;id=100&amp;file1=100-1.jpg&amp;file2=&amp;file3=&amp;name=GradE8\","
            "\"preview2\": null,"
            "\"preview3\": null,"
            "\"previewpic1\": \"https://www.KDE-Look.org/CONTENT/content-pre1/100-1.jpg\","
            "\"previewpic2\": null,"
            "\"previewpic3\": null,"
            "\"smallpreviewpic1\": \"https://www.KDE-Look.org/CONTENT/content-m1/m100-1.png\","
            "\"smallpreviewpic2\": null,"
            "\"smallpreviewpic3\": null,"
            "\"description\": \"This is my first KDE 2.0 theme. It is not the final version, I must add some icons etc...\","
            "\"changelog\": null,"
            "\"feedbackurl\": \"https://openDesktop.org/feedback\","
            "\"homepage\": \"https://en.wikipedia.org/foo111\","
            "\"homepagetype\": \"Wikipedia\","
            "\"homepage2\": null,"
            "\"homepagetype2\": null,"
            "\"homepage3\": null,"
            "\"homepagetype3\": null,"
            "\"homepage4\": null,"
            "\"homepagetype4\": null,"
            "\"homepage5\": null,"
            "\"homepagetype5\": null,"
            "\"homepage6\": null,"
            "\"homepagetype6\": null,"
            "\"homepage7\": null,"
            "\"homepagetype7\": null,"
            "\"homepage8\": null,"
            "\"homepagetype8\": null,"
            "\"homepage9\": null,"
            "\"homepagetype9\": null,"
            "\"homepage10\": null,"
            "\"homepagetype10\": null,"
            "\"donationpage\": \"https://www.opendesktop.org/content/donation.php?content=123\","

            "\"icon\": ["
                "{"
                    "\"width\": 16,"
                    "\"height\": 16,"
                    "\"link\": \"https://www.KDE-Look.org/img/icon1.png\""
                "},"
                "{"
                    "\"width\": 32,"
                    "\"height\": 32,"
                    "\"link\": \"https://www.KDE-Look.org/img/icon2.png\""
                "},"
                "{"
                    "\"width\": 64,"
                    "\"height\": 64,"
                    "\"link\": \"https://www.KDE-Look.org/img/icon2.png\""
                "}"
            "],"

            "\"video\": ["
                "{"
                    "\"link\": \"https://www.KDE-Look.org/video/video1.mpg\""
                "},"
                "{"
                    "\"link\": \"https://www.KDE-Look.org/video/video2.mpg\""
                "},"
                "{"
                    "\"link\": \"https://www.KDE-Look.org/video/video3.mpg\""
                "}"
            "],"

            "\"downloadway1\": 1,"
            "\"downloadtype1\": \"Fedora\","
            "\"downloadprice1\": 0,"
            "\"downloadlink1\": \"https://www.opendesktop.org/content/download.php?content=1423&amp;id=2\","
            "\"downloadname1\": \"gdfgd22\","
            "\"downloadsize1\": 2,"
            "\"downloadgpgsignature1\": \"iEYEABECAAYFAkxT52oACgkQMNASEGDVgdegPAbDSMHn/xDQCfSplogMr9x0G0ZNqMUAn3WLVmXADVzWdEToTJ8B5wpdm3zb=A6Dy\","
            "\"downloadgpgfingerprint1\": \"6AD9 150F D8CC 941B 4541  2DCC 68B7 AB89 5754 8D2D\","
            "\"downloadpackagename1\": \"packname\","
            "\"downloadrepository1\": \"repo\","
            "\"downloadtype2\": \"Fedora\","
            "\"downloadprice2\": 2.99,"
            "\"downloadlink2\": \"https://www.opendesktop.org/content/buy.php?content=1423&amp;id=1\","
            "\"downloadname2\": \"gdgg22\","
            "\"downloadgpgsignature2\": \"iEYEABECAAYFAkxT52oACgkQMNASEGDVgdegPAbDSMHn/xDQCfSplogMr9x0G0ZNqMUAn3WLVmXADVzWdEToTJ8B5wpdm3zb=A6Dy\","
            "\"downloadgpgfingerprint2\": \"6AD9 150F D8CC 941B 4541  2DCC 68B7 AB89 5754 8D2D\","
            "\"downloadpackagename1\": \"packname\","
            "\"downloadrepository1\": \"repo\","
            "\"detailpage\": \"https://www.KDE-Look.org/content/show.php?content=100\""
        "}"
        "]") + endString;
    JsonParser<Content> parser;
    parser.parse( testData );
    Content content = parser.item();

    QVERIFY( content.isValid() );
    QCOMPARE( content.id(), QLatin1String("100") );
    QCOMPARE( content.name(), QLatin1String("GradE8") );
    QCOMPARE( content.rating(), 67 );
    QCOMPARE( content.downloads(), 2 );
    QCOMPARE( content.numberOfComments(), 0 );
    QCOMPARE( content.created(), QDateTime::fromString(QLatin1String("2001-09-28T18:45:40+02:00"), Qt::ISODate) );
    QCOMPARE( content.updated(), QDateTime::fromString(QLatin1String("2001-09-28T18:45:40+02:00"), Qt::ISODate) );
    QCOMPARE( content.icons().size(), 3 );
    QCOMPARE( content.icons().at(1).width(), 32u );
    QCOMPARE( content.videos().size(), 3 );
    QCOMPARE( content.videos().at(0), QUrl( QLatin1String("https://www.KDE-Look.org/video/video1.mpg") ) );
}

void JsonTest::testDistribution()
{
    QString testData = startString + QLatin1String("["
        "{"
            "\"id\": 0,"
            "\"name\": null"
        "},"
        "{"
            "\"id\": 2200,"
            "\"name\": \"Arch\""
        "},"
        "{"
            "\"id\": 2000,"
            "\"name\": \"Ark\""
        "},"
        "{"
            "\"id\": 1100,"
            "\"name\": \"Debian\""
        "}"
        "]") + endString;
    JsonParser<Distribution> parser;
    parser.parse( testData );
    QCOMPARE( parser.itemList().size(), 4 );
    Distribution distribution = parser.itemList().at(1);

    QCOMPARE( distribution.id(), 2200u );
    QCOMPARE( distribution.name(), QLatin1String("Arch") );
}

void JsonTest::testDownloadItem()
{
    QString testData = startString + QLatin1String("["
        "{"
            "\"details\": \"download\","
            "\"downloadway\": 0,"
            "\"downloadlink\": \"https://www......tar.gz\","
            "\"mimetype\": \"image/jpeg\","
            "\"packagename\": \"glibc-2.10.1-10.4.i686.rpm\","
            "\"packagerepository\": \"https://download.opensuse.org/distribution/11.2/repo/oss/\","
            "\"gpgsignature\": \"iEYEABECAAYFAkxT52oACgkQMNASEGDVgdegPAbDSMHn/xDQCfSplogMr9x0G0ZNqMUAn3WLVmXADVzWdEToTJ8B5wpdm3zb=A6Dy\","
            "\"gpgfingerprint\": \"6AD9 150F D8CC 941B 4541  2DCC 68B7 AB89 5754 8D2D\""
        "}"
        "]") + endString;
    JsonParser<DownloadItem> parser;
    parser.parse( testData );
    DownloadItem item = parser.item();

    QCOMPARE( item.url(), QUrl( QLatin1String("https://www......tar.gz") ) );
    QCOMPARE( item.mimeType(), QLatin1String("image/jpeg") );
    QCOMPARE( item.packageName(), QLatin1String("glibc-2.10.1-10.4.i686.rpm") );
    QCOMPARE( item.packageRepository(), QLatin1String("https://download.opensuse.org/distribution/11.2/repo/oss/") );
    QCOMPARE( item.gpgFingerprint(), QLatin1String("6AD9 150F D8CC 941B 4541  2DCC 68B7 AB89 5754 8D2D") );
    QCOMPARE( item.gpgSignature(), QLatin1String("iEYEABECAAYFAkxT52oACgkQMNASEGDVgdegPAbDSMHn/xDQCfSplogMr9x0G0ZNqMUAn3WLVmXADVzWdEToTJ8B5wpdm3zb=A6Dy") );
    QCOMPARE( item.type(), DownloadDescription::FileDownload );
}

void JsonTest::testHomepageType()
{
    QString testData = startString + QLatin1String("["
        "{"
            "\"id\": 0,"
            "\"name\": \"&amp;nbsp;\""
        "},"
        "{"
            "\"id\": 10,"
            "\"name\": \"Blog\""
        "},"
        "{"
            "\"id\": 20,"
            "\"name\": \"Facebook\""
        "}"
        "]") + endString;
    JsonParser<HomePageType> parser;
    parser.parse( testData );
    QCOMPARE( parser.itemList().size(), 3 );
    HomePageType type = parser.itemList().at(1);

    QCOMPARE( type.id(), 10u );
    QCOMPARE( type.name(), QLatin1String("Blog") );
}

void JsonTest::testIcon()
{
    QString testData = startString + QLatin1String("["
        "{"
            "\"width\": 16,"
            "\"height\": 16,"
            "\"link\": \"https://www.KDE-Look.org/img/icon1.png\""
        "},"
        "{"
            "\"width\": 32,"
            "\"height\": 32,"
            "\"link\": \"https://www.KDE-Look.org/img/icon2.png\""
        "},"
        "{"
            "\"width\": 64,"
            "\"height\": 64,"
            "\"link\": \"https://www.KDE-Look.org/img/icon2.png\""
        "}"
        "]") + endString;
    JsonParser<Icon> parser;
    parser.parse( testData );
    QCOMPARE( parser.itemList().size(), 3 );
    Icon icon = parser.item();

    QCOMPARE( icon.width(), 16u );
    QCOMPARE( icon.height(), 16u );
    QCOMPARE( icon.url(), QUrl(QLatin1String("https://www.KDE-Look.org/img/icon1.png")) );
}

void JsonTest::testLicense()
{
    QString testData = startString + QLatin1String("["
        "{"
            "\"id\": 1000,"
            "\"name\": null,"
            "\"link\": null"
        "},"
        "{"
            "\"id\": 3,"
            "\"name\": \"Artistic 2.0\","
            "\"link\": \"https://dev.perl.org/perl6/rfc/346.html\""
        "},"
        "{"
            "\"id\": 6,"
            "\"name\": \"BSD\","
            "\"link\": \"https://www.opensource.org/licenses/bsd-license.php\""
        "}"
        "]") + endString;
    JsonParser<License> parser;
    parser.parse( testData );
    QCOMPARE( parser.itemList().size(), 3 );
    License license = parser.itemList().at(1);

    QCOMPARE( license.id(), 3u );
    QCOMPARE( license.name(), QLatin1String("Artistic 2.0") );
    QCOMPARE( license.url(), QUrl(QLatin1String("https://dev.perl.org/perl6/rfc/346.html")) );
}

void JsonTest::testMessage()
{
    QString testData1 =startString+QLatin1String("["
        "{"
            "\"details\": \"full\","
            "\"id\": 8490,"
            "\"message from\": \"testy\","
            "\"firstname\": \"Laszlo\","
            "\"lastname\": \"Papp\","
            "\"profilepage\": \"https://www.opendesktop.org/usermanager/search.php?username=lpapp\","
            "\"messageto\": \"lpapp\","
            "\"senddate\": \"2008-08-10T16:03:59+02:00\","
            "\"status\": 1,"
            "\"statustext\": null,"
            "\"subject\": \"test message\","
            "\"body\": \"Sorry for bothering but did you ...\""
        "}"
        "]")+endString;
    JsonParser<Message> parser;
    parser.parse( testData1 );
    Message message = parser.item();

    QVERIFY( message.isValid() );
    QCOMPARE( message.id(), QLatin1String("8490") );
    QCOMPARE( message.from(), QLatin1String("testy") );
    QCOMPARE( message.to(), QLatin1String("lpapp") );
    QCOMPARE( message.sent(), QDateTime::fromString(QLatin1String("2008-08-10T16:03:59+02:00"), Qt::ISODate) );
    QCOMPARE( message.status(), Message::Status(1) );
    QCOMPARE( message.subject(), QLatin1String("test message") );
    QCOMPARE( message.body(), QLatin1String("Sorry for bothering but did you ...") );
}


void JsonTest::testPerson()
{
    QString testData1 = startString+QLatin1String("["
        "{"
            "\"details\": \"full\","
            "\"personid\": \"lpapp\","
            "\"privacy\": 1,"
            "\"privacytext\": \"visible only for registered users\","
            "\"firstname\": \"Laszlo\","
            "\"lastname\": \"Papp\","
            "\"gender\": \"male\","
            "\"communityrole\": \"developer\","
            "\"homepage\": null,"
            "\"company\": \"company\","
            "\"avatarpic\": \"https://www.opendesktop.org/usermanager/nopic.png\","
            "\"avatarpicfound\": 1,"
            "\"bigavatarpic\": \"https://www.opendesktop.org/usermanager/nopic.png\","
            "\"bigavatarpicfound\": 1,"
            "\"birthday\": \"1985-12-02\","
            "\"jobstatus\": \"working\","
            "\"city\": \"Helsinki\","
            "\"country\": \"Finland\","
            "\"longitude\": 24.9375,"
            "\"latitude\": 60.1708,"
            "\"ircnick\": \"djszapi\","
            "\"ircchannels\": \"ocs, qt-labs\","

            "\"irclink\": ["
                "{"
                    "\"link\": \"irc://irc.freenode.org/ocs\""
                "},"
                "{"
                    "\"link\": \"irc://irc.freenode.org/qt-labs\""
                "}"
            "],"

            "\"likes\": \"lot of stuff\","
            "\"dontlikes\": \"nothing\","
            "\"interests\": \"travel\","
            "\"languages\": \"english\","
            "\"programminglanguages\": \"c, c++\","
            "\"favouritequote\": null,"
            "\"favouritemusic\": \"Iron Maiden\","
            "\"favouritetvshows\": null,"
            "\"favouritemovies\": \"Back to the Future\","
            "\"favouritebooks\": null,"
            "\"favouritegames\": \"Star Craft\","
            "\"description\": null,"
            "\"profilpage\": \"https://www.KDE-Look.org/usermanager/search.php?username=lpapp\""
        "}"
        "]") + endString;
    JsonParser<Person> parser;
    parser.parse( testData1 );
    Person person = parser.item();
    QVERIFY( person.isValid() );
    QCOMPARE( person.id(), QLatin1String("lpapp") );
    QCOMPARE( person.firstName(), QLatin1String("Laszlo") );
    QCOMPARE( person.lastName(), QLatin1String("Papp") );
    QCOMPARE( person.birthday(), QDate(1985, 12, 02) );
    QCOMPARE( person.country(), QLatin1String("Finland") );
    QCOMPARE( person.latitude(), 60.1708 );
    QCOMPARE( person.longitude(), 24.9375 );
    QCOMPARE( person.avatarUrl(), QUrl( QLatin1String("https://www.opendesktop.org/usermanager/nopic.png") ) );
    QCOMPARE( person.homepage(), QLatin1String("") );
    QCOMPARE( person.city(), QLatin1String("Helsinki") );
    QCOMPARE( person.extendedAttribute(QLatin1String("favouritemusic")), QLatin1String("Iron Maiden"));
}

// void JsonTest::testConfig()
// {
//     QString testData1 = startString +
//             "{"
//             "\"version\": \"1.4\","
//             "\"website\": \"opendesktop.org\","
//             "\"host\": \"api.opendesktop.org\","
//             "\"contact\": \"frank@opendesktop.org\","
//             "\"ssl\": true"
//             "}" + endString;
//     JsonParser parser;
//     parser.parse(testData1);
//     Config
// }

QTEST_MAIN(JsonTest);

#include "jsontest.moc"
