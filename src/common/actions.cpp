/*
 * (c) 2017, Deepin Technology Co., Ltd. <support@deepin.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
#include "config.h"
#include "actions.h"
#include "player_engine.h"

namespace dmr {

static ActionFactory *_factory = nullptr;

ActionFactory &ActionFactory::get()
{
    if (_factory == nullptr) {
        _factory = new ActionFactory();
    }

    return *_factory;
}

#define DEF_ACTION(NAME, KD) do { \
        auto *act = menu_p->addAction((NAME)); \
        act->setProperty("kind", KD); \
        _contextMenuActions.append(act); \
        connect(act, &QObject::destroyed, [=](QObject* o) { \
            _contextMenuActions.removeOne((QAction*)o); \
        }); \
    } while (0)

#define DEF_ACTION_CHECKED(NAME, KD) do { \
        auto *act = menu_p->addAction((NAME)); \
        act->setCheckable(true); \
        act->setProperty("kind", KD); \
        _contextMenuActions.append(act); \
        connect(act, &QObject::destroyed, [=](QObject* o) { \
            _contextMenuActions.removeOne((QAction*)o); \
        }); \
    } while (0)

#define DEF_ACTION_GROUP(NAME, KD, GROUP) do { \
        auto *act = menu->addAction((NAME)); \
        act->setProperty("kind", KD); \
        _contextMenuActions.append(act); \
        connect(act, &QObject::destroyed, [=](QObject* o) { \
            _contextMenuActions.removeOne((QAction*)o); \
        }); \
    } while (0)

#define DEF_ACTION_CHECKED_GROUP(NAME, KD, GROUP) do { \
        auto *act = menu->addAction((NAME)); \
        act->setCheckable(true); \
        act->setProperty("kind", KD); \
        act->setActionGroup(GROUP); \
        _contextMenuActions.append(act); \
        connect(act, &QObject::destroyed, [=](QObject* o) { \
            _contextMenuActions.removeOne((QAction*)o); \
        }); \
    } while (0)

DMenu *ActionFactory::titlebarMenu()
{
    if (!_titlebarMenu) {
        auto *menu_p = new DMenu();

        DEF_ACTION(tr("Open file"), ActionKind::OpenFileList);
        DEF_ACTION(tr("Open folder"), ActionKind::OpenDirectory);
        DEF_ACTION(tr("Settings"), ActionKind::Settings);
//        DEF_ACTION_CHECKED(tr("Light theme"), ActionKind::LightTheme);
        menu_p->addSeparator();
        // these seems added by titlebar itself
        //DEF_ACTION("About", ActionKind::About);
        //DEF_ACTION("Help", ActionKind::Help);
        //DEF_ACTION("Exit", ActionKind::Exit);

        _titlebarMenu = menu_p;
    }
    return _titlebarMenu;
}

DMenu *ActionFactory::mainContextMenu()
{
    if (!_contextMenu) {
        auto *menu_p = new DMenu();

        DEF_ACTION(tr("Open file"), ActionKind::OpenFileList);
        DEF_ACTION(tr("Open folder"), ActionKind::OpenDirectory);
        DEF_ACTION(tr("Open URL"), ActionKind::OpenUrl);
        DEF_ACTION(tr("Open CD/DVD"), ActionKind::OpenCdrom);
        menu_p->addSeparator();

        DEF_ACTION_CHECKED(tr("Fullscreen"), ActionKind::ToggleFullscreen);
        DEF_ACTION_CHECKED(tr("Mini Mode"), ActionKind::ToggleMiniMode);
        DEF_ACTION_CHECKED(tr("Always on Top"), ActionKind::WindowAbove);
        menu_p->addSeparator();

        {
            auto *parent = menu_p;
            auto *menu = new DMenu(tr("Play Mode"));
            auto group = new QActionGroup(menu);

            DEF_ACTION_CHECKED_GROUP(tr("Order Play"), ActionKind::OrderPlay, group);
            DEF_ACTION_CHECKED_GROUP(tr("Shuffle Play"), ActionKind::ShufflePlay, group);
            DEF_ACTION_CHECKED_GROUP(tr("Single Play"), ActionKind::SinglePlay, group);
            DEF_ACTION_CHECKED_GROUP(tr("Single Loop"), ActionKind::SingleLoop, group);
            DEF_ACTION_CHECKED_GROUP(tr("List Loop"), ActionKind::ListLoop, group);

            parent->addMenu(menu);
        }

        {
            auto *parent = menu_p;
            auto *menu = new DMenu(tr("Playback Speed"));
            auto group = new QActionGroup(menu);

            DEF_ACTION_CHECKED_GROUP(tr("0.5x"), ActionKind::ZeroPointFiveTimes, group);
            DEF_ACTION_CHECKED_GROUP(tr("1.0x"), ActionKind::OneTimes, group);
            DEF_ACTION_CHECKED_GROUP(tr("1.2x"), ActionKind::OnePointTwoTimes, group);
            DEF_ACTION_CHECKED_GROUP(tr("1.5x"), ActionKind::OnePointFiveTimes, group);
            DEF_ACTION_CHECKED_GROUP(tr("2.0x"), ActionKind::Double, group);

            parent->addMenu(menu);
            menu->setEnabled(false);
            connect(this, &ActionFactory::playSpeedMenuEnable, this, [ = ](bool statu) {
                menu->setEnabled(statu);
            });
        }

        {
            auto *parent = menu_p;
            auto *menu = new DMenu(tr("Frame"));
            auto group = new QActionGroup(menu);

            DEF_ACTION_CHECKED_GROUP(tr("Default"), ActionKind::DefaultFrame, group);
            DEF_ACTION_CHECKED_GROUP(("4:3"), ActionKind::Ratio4x3Frame, group);
            DEF_ACTION_CHECKED_GROUP(("16:9"), ActionKind::Ratio16x9Frame, group);
            DEF_ACTION_CHECKED_GROUP(("16:10"), ActionKind::Ratio16x10Frame, group);
            DEF_ACTION_CHECKED_GROUP(("1.85:1"), ActionKind::Ratio185x1Frame, group);
            DEF_ACTION_CHECKED_GROUP(("2.35:1"), ActionKind::Ratio235x1Frame, group);
            menu->addSeparator();

            DEF_ACTION_GROUP(tr("Clockwise"), ActionKind::ClockwiseFrame, group);
            DEF_ACTION_GROUP(tr("Counterclockwise"), ActionKind::CounterclockwiseFrame, group);
            menu->addSeparator();

            DEF_ACTION_GROUP(tr("Next Frame"), ActionKind::NextFrame, group);
            DEF_ACTION_GROUP(tr("Previous Frame"), ActionKind::PreviousFrame, group);

            parent->addMenu(menu);
            menu->setEnabled(false);
            connect(this, &ActionFactory::frameMenuEnable, this, [ = ](bool statu) {
                menu->setEnabled(statu);
            });
        }

        {
            //sound menu
            auto *parent = menu_p;
            auto *menu = new DMenu(tr("Sound"));
            _sound = menu;
            {
                auto *parent_channel = menu;
                auto *menu = new DMenu(tr("Channel"));
                _soundMenu = menu;
                auto group = new QActionGroup(menu);

                DEF_ACTION_CHECKED_GROUP(tr("Stereo"), ActionKind::Stereo, group);
                DEF_ACTION_CHECKED_GROUP(tr("Left channel"), ActionKind::LeftChannel, group);
                DEF_ACTION_CHECKED_GROUP(tr("Right channel"), ActionKind::RightChannel, group);
                parent_channel->addMenu(menu);
            }

            {
                auto *parent_track = menu;
                auto *menu = new DMenu(tr("Track"));
                _tracksMenu = menu;
                //DEF_ACTION(tr("Select Track"), ActionKind::SelectTrack);
                parent_track->addMenu(menu);
            }
            parent->addMenu(menu);
        }

        {
            //sub menu
            auto *parent = menu_p;
            auto *menu = new DMenu(tr("Subtitle"));
            auto group = new QActionGroup(menu);

            DEF_ACTION_GROUP(tr("Load"), ActionKind::LoadSubtitle, group);
            DEF_ACTION_GROUP(tr("Online Search"), ActionKind::MatchOnlineSubtitle, group);
            //DEF_ACTION(tr("Select"), ActionKind::SelectSubtitle);
            {
                auto *parent_select = menu;
                auto *menu = new DMenu(tr("Select"));
                _subtitleMenu = menu;
                parent_select->addMenu(menu);
            }
            DEF_ACTION_CHECKED_GROUP(tr("Hide"), ActionKind::HideSubtitle, group);

            {
                auto *parent_encoding = menu;
                auto *menu = new DMenu(tr("Encodings"));
                auto group_encoding = new QActionGroup(menu);

                //title <-> codepage
                static QVector<QPair<QString, QString>> list = {
                    {"Auto", "auto"},
                    {"Universal (UTF-8)", "UTF-8"},
                    {"Universal (UTF-16)", "UTF-16"},
                    {"Universal (UTF-16BE)", "UTF-16BE"},
                    {"Universal (UTF-16LE)", "UTF-16LE"},
                    {"Arabic (ISO-8859-6)", "ISO-8859-6"},
                    {"Arabic (WINDOWS-1256)", "WINDOWS-1256"},
                    {"Baltic (LATIN7)", "LATIN7"},
                    {"Baltic (WINDOWS-1257)", "WINDOWS-1257"},
                    {"Celtic (LATIN8)", "LATIN8"},
                    {"Central European (WINDOWS-1250)", "WINDOWS-1250"},
                    {"Cyrillic (ISO-8859-5)", "ISO-8859-5"},
                    {"Cyrillic (WINDOWS-1251)", "WINDOWS-1251"},
                    {"Eastern European (ISO-8859-2)", "ISO-8859-2"},
                    {"Western Languages (WINDOWS-1252)", "WINDOWS-1252"},
                    {"Greek (ISO-8859-7)", "ISO-8859-7"},
                    {"Greek (WINDOWS-1253)", "WINDOWS-1253"},
                    {"Hebrew (ISO-8859-8)", "ISO-8859-8"},
                    {"Hebrew (WINDOWS-1255)", "WINDOWS-1255"},
                    {"Japanese (SHIFT-JIS)", "SHIFT-JIS"},
                    {"Japanese (ISO-2022-JP-2)", "ISO-2022-JP-2"},
                    {"Korean (EUC-KR)", "EUC-KR"},
                    {"Korean (CP949)", "CP949"},
                    {"Korean (ISO-2022-KR)", "ISO-2022-KR"},
                    {"Nordic (LATIN6)", "LATIN6"},
                    {"North European (LATIN4)", "LATIN4"},
                    {"Russian (KOI8-R)", "KOI8-R"},
                    {"Simplified Chinese (GBK)", "GBK"},
                    {"Simplified Chinese (GB18030)", "GB18030"},
                    {"Simplified Chinese (ISO-2022-CN-EXT)", "ISO-2022-CN-EXT"},
                    {"South European (LATIN3)", "LATIN3"},
                    {"South-Eastern European (LATIN10)", "LATIN10"},
                    {"Thai (TIS-620)", "TIS-620"},
                    {"Thai (WINDOWS-874)", "WINDOWS-874"},
                    {"Traditional Chinese (EUC-TW)", "EUC-TW"},
                    {"Traditional Chinese (BIG5)", "BIG5"},
                    {"Traditional Chinese (BIG5-HKSCS)", "BIG5-HKSCS"},
                    {"Turkish (LATIN5)", "LATIN5"},
                    {"Turkish (WINDOWS-1254)", "WINDOWS-1254"},
                    {"Ukrainian (KOI8-U)", "KOI8-U"},
                    {"Vietnamese (WINDOWS-1258)", "WINDOWS-1258"},
                    {"Vietnamese (VISCII)", "VISCII"},
                    {"Western European (LATIN1)", "LATIN1"},
                    {"Western European (LATIN-9)", "LATIN-9"}
                };

                auto p = list.begin();
                while (p != list.end()) {
                    DEF_ACTION_CHECKED_GROUP(p->first, ActionKind::ChangeSubCodepage, group_encoding);
                    auto act = menu->actions().last();
                    act->setProperty("args", QList<QVariant>() << p->second);
                    if (p->second == "auto") menu->addSeparator();
                    p++;
                }

                parent_encoding->addMenu(menu);
            }

            parent->addMenu(menu);
        }

        {
            //sub menu
            auto *parent = menu_p;
            auto *menu = new DMenu(tr("Screenshot"));
            auto group = new QActionGroup(menu);

            DEF_ACTION_GROUP(tr("Film Screenshot"), ActionKind::Screenshot, group);
            DEF_ACTION_GROUP(tr("Burst Shooting"), ActionKind::BurstScreenshot, group);
            DEF_ACTION_GROUP(tr("Open screenshot folder"), ActionKind::GoToScreenshotSolder, group);
            menu->setEnabled(false);
            parent->addMenu(menu);
            connect(this, &ActionFactory::frameMenuEnable, this, [ = ](bool statu) {
                menu->setEnabled(statu);
            });
        }

        menu_p->addSeparator();

        DEF_ACTION_CHECKED(tr("Playlist"), ActionKind::TogglePlaylist);
        DEF_ACTION(tr("Film Info"), ActionKind::MovieInfo);
        DEF_ACTION(tr("Settings"), ActionKind::Settings);

        _contextMenu = menu_p;
    }

    return _contextMenu;
}

DMenu *ActionFactory::playlistContextMenu()
{
    if (!_playlistMenu) {
        auto *menu_p = new DMenu();


        DEF_ACTION(tr("Delete from playlist"), ActionKind::PlaylistRemoveItem);
        DEF_ACTION(tr("Empty playlist"), ActionKind::EmptyPlaylist);
        DEF_ACTION(tr("Display in file manager"), ActionKind::PlaylistOpenItemInFM);
        DEF_ACTION(tr("Film info"), ActionKind::PlaylistItemInfo);

        _playlistMenu = menu_p;
    }

    return _playlistMenu;

}

QList<QAction *> ActionFactory::findActionsByKind(ActionKind target_kd)
{
    QList<QAction *> res;
    auto p = _contextMenuActions.begin();
    while (p != _contextMenuActions.end()) {
#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
        auto kd = (ActionKind)(*p)->property("kind").value<int>();
#else
        auto kd = (*p)->property("kind").value<ActionKind>();
#endif
        if (kd == target_kd) {
            res.append(*p);
        }
        ++p;
    }
    return res;
}

void ActionFactory::updateMainActionsForMovie(const PlayingMovieInfo &pmf)
{
    qDebug() << __func__;
    if (_subtitleMenu) {
        auto menu = _subtitleMenu;
        menu->clear();

        if (!subgroup) {
            subgroup = new QActionGroup(menu); // mem leak ?
        }
        for (int i = 0; i < pmf.subs.size(); i++) {
            DEF_ACTION_CHECKED_GROUP(pmf.subs[i]["title"].toString(), ActionKind::SelectSubtitle, subgroup);
            auto act = menu->actions().last();
            act->setProperty("args", QList<QVariant>() << i);
        }

        _subtitleMenu->setEnabled(pmf.subs.size() > 0);
    }

    if (_subtitleMenu) {
        auto menu = _tracksMenu;
        menu->clear();

        if (!audiosgroup) {
            audiosgroup = new QActionGroup(menu); // mem leak ?
        }
        for (int i = 0; i < pmf.audios.size(); i++) {
            if (pmf.audios[i]["title"].toString().compare("[internal]") == 0) {
                DEF_ACTION_CHECKED_GROUP(tr("Track") + QString::number(i + 1), ActionKind::SelectTrack, audiosgroup);
            } else {
                DEF_ACTION_CHECKED_GROUP(pmf.audios[i]["title"].toString(), ActionKind::SelectTrack, audiosgroup);
            }
            auto act = menu->actions().last();
            act->setProperty("args", QList<QVariant>() << i);
        }

        _tracksMenu->setEnabled(pmf.audios.size() > 0);
        _soundMenu->setEnabled(pmf.audios.size() > 0);
        _sound->setEnabled(pmf.audios.size() > 0);
    }
}

#undef DEF_ACTION
#undef DEF_ACTION_CHECKED
}
