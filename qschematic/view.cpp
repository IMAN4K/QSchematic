#include <QKeyEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QAction>
#include <QMenu>
#include "commands/commanditemremove.h"
#include "view.h"
#include "scene.h"
#include "settings.h"
#include <QApplication>

const qreal ZOOM_FACTOR_MIN   = 0.25;
const qreal ZOOM_FACTOR_MAX   = 10.00;
const qreal ZOOM_FACTOR_STEPS = 0.10;
const qreal FIT_ALL_PADDING   = 20.00;

using namespace QSchematic;

View::View(QWidget* parent) :
    QGraphicsView(parent),
    _scene(nullptr),
    _scaleFactor(1.0),
    _mode(NormalMode)
{
    // Scroll bars
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // Interaction stuff
    setMouseTracking(true);
    setAcceptDrops(true);
    setDragMode(QGraphicsView::RubberBandDrag);

    // Rendering options
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
}

void View::keyPressEvent(QKeyEvent* event)
{
    // Something with CTRL held down?
    if (event->modifiers() & Qt::ControlModifier) {

        switch (event->key()) {
        case Qt::Key_Plus:
            _scaleFactor += ZOOM_FACTOR_STEPS;
            updateScale();
            return;

        case Qt::Key_Minus:
            _scaleFactor -= ZOOM_FACTOR_STEPS;
            updateScale();
            return;

        case Qt::Key_0:
            _scaleFactor = 1.0;
            updateScale();
            return;

        case Qt::Key_W:
            if (_scene) {
                _scene->setMode(Scene::WireMode);
            }
            return;

        case Qt::Key_Space:
            if (_scene) {
                _scene->toggleWirePosture();
            }
            return;

        default:
            break;
        }
    }

    // Just a key alone?
    switch (event->key()) {
    case Qt::Key_Escape:
        if (_scene) {
            _scene->setMode(Scene::NormalMode);
        }
        return;

    case Qt::Key_Delete:
        if (_scene) {
            for (auto item : _scene->selectedTopLevelItems()) {
                _scene->undoStack()->push(new CommandItemRemove(_scene, item));
            }
        }
        return;

    default:
        break;
    }

    // Fall back
    QGraphicsView::keyPressEvent(event);
}

void View::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->angleDelta().y() > 0 && _scaleFactor < ZOOM_FACTOR_MAX) {
            _scaleFactor += ZOOM_FACTOR_STEPS;
        } else if (event->angleDelta().y() < 0 && _scaleFactor > ZOOM_FACTOR_MIN) {
            _scaleFactor -= ZOOM_FACTOR_STEPS;
        }

        updateScale();
    }
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);

    switch (_mode) {
    case NormalMode:
        break;

    case PanMode:
        qDebug() << "====";
        qDebug() << event->pos();
        QPointF currentOffset = mapToScene(viewport()->geometry().center()) - _scene->itemsBoundingRect().center();
        QPointF offset(_panStart - event->pos());
        _offset = currentOffset + offset;
//        qDebug() << "current" << currentOffset.x() << "offset" << offset.x() << "new" << _offset.x();
//        qDebug() << _offset.x();
        QRectF oldSceneRect = sceneRect();
        qreal horizontalPos = horizontalScrollBar()->value() - (event->x() - _panStart.x());
//        qDebug() << horizontalScrollBar()->value();
//        qDebug() << horizontalPos;
        qreal verticalPos = verticalScrollBar()->value() - (event->y() - _panStart.y());
        updateRect();
//        qDebug() << currentOffset;
        qDebug() << mapToScene(offset.toPoint());
//        qDebug() << _offset;
        qDebug() << sceneRect().width() - oldSceneRect.width();
//        qDebug() << sceneRect().width() - mapToScene(viewport()->geometry()).boundingRect().width();

//        qDebug() << offset;
//        qDebug() << mapToScene(viewport()->geometry().center());
//        qDebug() << mapToScene(viewport()->geometry().center()) + offset;
//        centerOn(mapToScene(viewport()->geometry().center() + offset.toPoint()));
        qDebug() << horizontalScrollBar()->value();
        horizontalScrollBar()->setValue(horizontalPos);
        qDebug() << horizontalScrollBar()->value();
        verticalScrollBar()->setValue(verticalPos);
//        qDebug() << mapToScene(viewport()->geometry().center());

        _panStart = event->pos();
        event->accept();
        return;
    }
}

void View::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        setMode(PanMode);
        _panStart = event->pos();
        viewport()->setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

void View::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        setMode(NormalMode);
        viewport()->setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void View::setScene(Scene* scene)
{
    if (scene) {
        scene->setSceneRect(viewport()->geometry());
        connect(scene, &Scene::modeChanged, [this](int newMode){
            switch (newMode) {
            case Scene::NormalMode:
                viewport()->setCursor(Qt::ArrowCursor);
                break;

            case Scene::WireMode:
                viewport()->setCursor(Qt::CrossCursor);
                break;

            default:
                break;
            }
        });

        connect(scene, &Scene::sceneRectChanged, [=] {
            updateRect();
        });
    }

    QGraphicsView::setScene(scene);

    _scene = scene;
    updateRect();
    centerOn(sceneRect().center());
}

void View::updateRect()
{
    if (not _scene) {
        return;
    }

    QRectF itemsRect = _scene->itemsBoundingRect();
    QRectF viewRect = mapToScene(viewport()->geometry()).boundingRect();//.adjusted(0, 0, -2, -2);
//    QRectF viewRect = viewport()->geometry().adjusted(0, 0, -2, -2);
    QRectF currentRect = viewRect;
    currentRect.moveCenter(itemsRect.center() + _offset);
    viewRect.moveTo(itemsRect.center().x() - viewRect.width() / 2, itemsRect.center().y() - viewRect.height() / 2);
//    qDebug() << "Items" << itemsRect << "view size" << viewRect << "viewport" << currentRect;
    setSceneRect(viewRect.united(itemsRect).united(currentRect));
}

void View::setSettings(const Settings& settings)
{
    _settings = settings;

    // Rendering options
    setRenderHint(QPainter::Antialiasing, _settings.antialiasing);
}

void View::setZoomValue(qreal factor)
{
    _scaleFactor = factor;

    updateScale();
}

void View::updateScale()
{
    // Apply the new scale
    setTransform(QTransform::fromScale(_scaleFactor, _scaleFactor));
    updateRect();

    emit zoomChanged(_scaleFactor);
}

void View::setMode(Mode newMode)
{
    _mode = newMode;

    emit modeChanged(_mode);
}

qreal View::zoomValue() const
{
    return _scaleFactor;
}

void View::fitInView()
{
    // Check if there is a scene
    if (not _scene) {
        return;
    }

    // Find the combined bounding rect of all the items
    QRectF rect;
    for (const auto& item : _scene->QGraphicsScene::items()) {
        QRectF boundingRect = item->boundingRect();
        boundingRect.moveTo(item->scenePos());
        rect = rect.united(boundingRect);
    }

    // Add some padding
    const auto& adj = std::max(0.0, FIT_ALL_PADDING);
    rect.adjust(-adj, -adj, adj, adj);

    // Update and cap the scale factor
    qreal currentScaleFactor = _scaleFactor;
    QGraphicsView::fitInView(rect, Qt::KeepAspectRatio);
    _scaleFactor = viewport()->geometry().width() / mapToScene(viewport()->geometry()).boundingRect().width();
    if (currentScaleFactor < 1) {
        _scaleFactor = std::min(_scaleFactor, 1.0);
    } else {
        _scaleFactor = std::min(_scaleFactor, currentScaleFactor);
    }
    updateScale();
}
