#include "playlist_widget.h"
#include "playlist_model.h"
#include "compositing_manager.h"
#include "mpv_proxy.h"
#include "actions.h"
#include "mainwindow.h"
#include "utils.h"

#include <DApplication>
#include <dimagebutton.h>

namespace dmr {
enum ItemState {
    Normal,
    Playing,
    Hover,
};

class PlayItemWidget: public QFrame {
    Q_OBJECT
    Q_PROPERTY(QString bg READ getBg WRITE setBg DESIGNABLE true)
public:
    friend class PlaylistWidget;

    //FIXME: what if item destroyed
    PlayItemWidget(PlayItemInfo pif, QWidget* parent = 0)
        : QFrame(parent), _pif {pif} 
    {
        setProperty("PlayItemThumb", "true");
        setFrameShape(QFrame::NoFrame);

        // it's the same for all themes
        _play = QPixmap(":/resources/icons/dark/normal/film-top.png");

        setFixedHeight(67);
        auto *l = new QHBoxLayout(this);
        l->setContentsMargins(0, 0, 0, 0);
        setLayout(l);

        _thumb = new QLabel(this);
        setBg(QString(":/resources/icons/%1/normal/film-bg.png").arg(qApp->theme())); 
        l->addWidget(_thumb);

        auto *vl = new QVBoxLayout;
        l->addLayout(vl, 1);

        auto w = new QLabel(this);
        w->setProperty("Name", true);
        w->setText(pif.info.fileName());
        w->setWordWrap(true);
        vl->addWidget(w);

        w = new QLabel(this);
        w->setProperty("Time", true);
        w->setText(_pif.mi.durationStr());
        vl->addWidget(w);

        _closeBtn = new DImageButton(this);
        _closeBtn->setObjectName("CloseBtn");
        _closeBtn->show();
        _closeBtn->raise();
        connect(_closeBtn, &DImageButton::clicked, this, &PlayItemWidget::closeButtonClicked);
    }

    void setState(ItemState is) {
        setProperty("ItemState", is);
    }

    QString getBg() const { return _bg; }
    void setBg(const QString& s) 
    { 
        _bg = s; 

        QPixmap pm(s);
        QPainter p(&pm);

        if (!_pif.thumbnail.isNull()) {
            p.drawPixmap((pm.width() - _pif.thumbnail.width())/2, 
                    (pm.height() - _pif.thumbnail.height())/2, _pif.thumbnail);
        }
        p.drawPixmap((pm.width() - _play.width())/2, 
                (pm.height() - _play.height())/2, _play);
        p.end();

        _thumb->setPixmap(pm);
    }

signals:
    void closeButtonClicked();

protected:
    void mouseReleaseEvent(QMouseEvent* me) override 
    {
        qDebug() << __func__;
    }

    void resizeEvent(QResizeEvent* se) override
    {
        qDebug() << __func__;
        auto sz = this->size();
        _closeBtn->move(sz.width() - _closeBtn->width(), (sz.height() - _closeBtn->height())/2);
        //_closeBtn->move(sz.width() - 20, (sz.height() - _closeBtn->height())/2);
    }

    void mouseDoubleClickEvent(QMouseEvent* me) override
    {
        qDebug() << __func__;
    }

private:
    QString _bg;
    QLabel *_thumb;
    QPixmap _play;
    PlayItemInfo _pif;
    DImageButton *_closeBtn;


};

PlaylistWidget::PlaylistWidget(QWidget *mw, MpvProxy *mpv)
    :QFrame(mw), _mpv(mpv), _mw(static_cast<MainWindow*>(mw))
{
    bool composited = CompositingManager::get().composited();
    setFrameShape(QFrame::NoFrame);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_TranslucentBackground, false);
    if (!composited) {
        setWindowFlags(Qt::FramelessWindowHint|Qt::BypassWindowManagerHint);
        setContentsMargins(0, 0, 0, 0);
        setAttribute(Qt::WA_NativeWindow);
    }

    auto *l = new QVBoxLayout(this);
    setLayout(l);

    connect(&_mpv->playlist(), &PlaylistModel::countChanged, this, &PlaylistWidget::loadPlaylist);
    connect(&_mpv->playlist(), &PlaylistModel::currentChanged, this, &PlaylistWidget::updateItemStates);
}

PlaylistWidget::~PlaylistWidget()
{
}

void PlaylistWidget::updateItemStates()
{
    qDebug() << __func__;
    for (int i = 0; i < _items.size(); i++) {
        auto item = dynamic_cast<PlayItemWidget*>(_items.at(i));
        item->setState(ItemState::Normal);

        if (_mouseItem == item) {
            item->setState(ItemState::Hover);
        }

        if (i == _mpv->playlist().current()) {
            item->setState(ItemState::Playing);
        }

        //TODO: optimize, check if state is really updated
        item->style()->unpolish(item);
        item->style()->polish(item);
    }

}

void PlaylistWidget::openItemInFM()
{
    if (!_mouseItem) return;
    auto item = dynamic_cast<PlayItemWidget*>(_mouseItem);
    if (item) {
        utils::ShowInFileManager(item->_pif.mi.filePath);
    }
}

void PlaylistWidget::removeClickedItem()
{
    if (!_clickedItem) return;
    auto item = dynamic_cast<PlayItemWidget*>(_clickedItem);
    if (item) {
        qDebug() << __func__;
        _mpv->playlist().remove(_items.indexOf(_clickedItem));
    }
}

void PlaylistWidget::contextMenuEvent(QContextMenuEvent *cme)
{
    bool on_item = false;
    _mouseItem = nullptr;

    QPoint p = cme->pos();
    for (auto w: _items) {
        if (w->geometry().contains(p)) {
            _mouseItem = w;
            on_item = true;
            break;
        }
    }

    updateItemStates();

    auto menu = ActionFactory::get().playlistContextMenu();
    for (auto act: menu->actions()) {
        auto prop = (ActionKind)act->property("kind").toInt();
        if (prop == ActionKind::MovieInfo || 
                prop == ActionKind::PlaylistOpenItemInFM) {
            act->setEnabled(on_item);
        }
    }

    ActionFactory::get().playlistContextMenu()->popup(cme->globalPos());
}

void PlaylistWidget::loadPlaylist()
{
    {
        for(auto p: _items) {
            p->deleteLater();
        }
        _items.clear();

        QLayoutItem *child;
        while ((child = layout()->takeAt(0)) != 0) {
            delete child;
        }
    }

    if (!_mapper) {
        _mapper = new QSignalMapper(this);
        connect(_mapper, static_cast<void(QSignalMapper::*)(QWidget*)>(&QSignalMapper::mapped),
            [=](QWidget* w) {
                qDebug() << "item close clicked";
                _clickedItem = w;
                _mw->requestAction(ActionKind::PlaylistRemoveItem);
            });
    }

    auto items = _mpv->playlist().items();
    auto p = items.begin();
    while (p != items.end()) {
        auto w = new PlayItemWidget(*p, this);
        _items.append(w);
        layout()->addWidget(w);

        connect(w, SIGNAL(closeButtonClicked()), _mapper, SLOT(map()));
        _mapper->setMapping(w, w);
        ++p;
    }
    static_cast<QVBoxLayout*>(layout())->addStretch(1);

    updateItemStates();
}

void PlaylistWidget::togglePopup()
{
    this->setVisible(!isVisible());
}


}

#include "playlist_widget.moc"