#include <QKeyEvent>

#include <GL/glew.h>

#include "GlTestFlcWidget.h"
#include <vector>
#include "qdebug.h"

#include "FastLaplaceComper.h"

#include <QTime>

#include <highgui.h>

#define glError() { \
        GLenum err = glGetError(); \
        while (err != GL_NO_ERROR) { \
                fprintf(stderr, "glError: %s caught at %s:%u\n", (char *)gluErrorString(err), __FILE__, __LINE__); \
                err = glGetError(); \
        } \
}


#define ROT_KEY Qt::Key_R
#define TRANS_KEY Qt::Key_T
#define SCALE_KEY Qt::Key_S
#define NO_KEY Qt::Key_unknown


GlTestFlcWidget::GlTestFlcWidget(QWidget *parent)
    : QGLWidget(parent)
{
    m_curKey = Qt::Key_unknown;
    m_interactionState = NO_SELECTION;

    m_drawmesh = false;
}

void GlTestFlcWidget::initializeGL()
{
    GLenum err = glewInit();

    m_flc = new FastLaplaceComper;

    m_rotate_x = m_rotate_y = 0;

    IplImage *img_ = cvLoadImage("./mona_ginerva.jpg");
    IplImage *img = cvCreateImage(cvSize(img_->width/3, img_->height/3), IPL_DEPTH_8U, 3);
    cvResize(img_, img);
    cvFlip(img);
    m_flc->SetSourceImage(img);
    m_flc->SetTargetImage(img);

    m_doInterp = true;

    resize(img->width, img->height);
}


void GlTestFlcWidget::paintGL()
{
    if(m_interactionState == SELECTING)
    {
        glDisable(GL_TEXTURE_2D);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, width(), height(), 0, -1, 1);
        glViewport(0, 0, width(), height());

        glColor3f(1,1,1);
        glBegin(GL_LINES);

            glVertex3f(m_selectlet1.x(), m_selectlet1.y(), 0);
            glVertex3f(m_selectlet2.x(), m_selectlet2.y(), 0);
        glEnd();

        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glEnable(GL_TEXTURE_2D);
        return;
    }

    glClearColor(1,0,0,1);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glError();

    glEnable(GL_TEXTURE_2D);
    m_flc->Render();

    return;
}

void GlTestFlcWidget::showEvent(QShowEvent *event)
{
    grabKeyboard();
    grabMouse();
}

void GlTestFlcWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
}

void GlTestFlcWidget::mousePressEvent(QMouseEvent *event)
{
    m_drag_start = QPoint(event->pos().x(), height()-event->pos().y());
    m_last_pos = event->pos();

    switch(m_interactionState)
    {
    case NO_SELECTION:
        mousepress_no_selection(event);
    break;
    case SELECTING:
        // shouldn't happen
    break;
    case REGION_SELECTED:
        mousepress_region_selected(event);
    break;
    case ROTATING:
        // shouldn't happen
    break;
    case TRANSLATING:
        // shouldn't happen
    break;
    case SCALING:
        // shouldn't happen
    break;
    }
}

void GlTestFlcWidget::mousepress_region_selected(QMouseEvent *event)
{
    switch(m_curKey)
    {
    case ROT_KEY:
        m_interactionState = ROTATING;
    break;
    case SCALE_KEY:
        m_interactionState = SCALING;
    break;
    case TRANS_KEY:
        m_interactionState = TRANSLATING;
    break;
    case NO_KEY:
        m_interactionState = SELECTING;
        mousepress_no_selection(event);
    break;
    default:
        ;
    }
}

void GlTestFlcWidget::mousepress_no_selection(QMouseEvent *event)
{
    m_lassoPts.clear();
    m_lassoPts.push_back(QPoint(event->pos().x(), height()-event->pos().y()));

    m_interactionState = SELECTING;
}

void GlTestFlcWidget::mouseMoveEvent(QMouseEvent *event)
{
    //m_drag_offset = event->pos() - m_drag_start;

    switch(m_interactionState)
    {
    case NO_SELECTION:
        // nothing to do
    break;
    case SELECTING:
        mousemove_selecting(event);
    break;
    case REGION_SELECTED:
        mousemove_region_selected(event);
    break;
    case ROTATING:
        mousemove_rotating(event);
    break;
    case TRANSLATING:
        mousemove_translating(event);
    break;
    case SCALING:
        mousemove_scaling(event);
    break;
    }

    //updateGL();
}

void GlTestFlcWidget::mousemove_rotating(QMouseEvent *event)
{
    QPoint dragVec = QPoint(event->pos().x(), height()-event->pos().y()) - m_drag_start;

    double rotation = dragVec.x();

    m_flc->SetTransform(rotation, pt_t(m_drag_start.x(), m_drag_start.y()), 1, pt_t(), false);
    updateGL();
}

void GlTestFlcWidget::mousemove_translating(QMouseEvent *event)
{
    QPoint translation = QPoint(event->pos().x(), height()-event->pos().y()) - m_drag_start;

    m_flc->SetTransform(0, pt_t(), 1, pt_t(translation.x(), translation.y()), false);
    updateGL();
}

void GlTestFlcWidget::mousemove_scaling(QMouseEvent *event)
{
    QPoint dragVec = QPoint(event->pos().x(), height()-event->pos().y()) - m_drag_start;

    double scale = dragVec.x()/10.;

    m_flc->SetTransform(0, pt_t(), scale, pt_t(), false);
    updateGL();
}


void GlTestFlcWidget::mousemove_selecting(QMouseEvent *event)
{
    m_lassoPts.push_back(QPoint(event->pos().x(), height()-event->pos().y()));

    m_selectlet1 = m_last_pos;
    m_selectlet2 = event->pos();

    m_last_pos = event->pos();
    updateGL();
}

void GlTestFlcWidget::mousemove_region_selected(QMouseEvent *event)
{

}



void GlTestFlcWidget::mouseReleaseEvent(QMouseEvent *event)
{
//    m_rotate_x += m_drag_offset.x();
//    m_rotate_y += m_drag_offset.y();
//    m_drag_offset = QPoint();
//    update();


    switch(m_interactionState)
    {
    case NO_SELECTION:
        // shouldn't get here
    break;
    case SELECTING:
        mouserelease_selecting(event);
    break;
    case REGION_SELECTED:
        mouserelease_region_selected(event);
    break;
    case ROTATING:
        mouserelease_rotating(event);
    break;
    case TRANSLATING:
        mouserelease_translating(event);
    break;
    case SCALING:
        mouserelease_scaling(event);
    break;
    }

}

void GlTestFlcWidget::mouserelease_selecting(QMouseEvent *event)
{
    m_lassoPts.push_back(QPoint(event->pos().x(), height()-event->pos().y()));
    m_interactionState = REGION_SELECTED;

    //qDebug() << "mouserelease_selecting() -- selected this contour: " << m_lassoPts;

    std::vector<pt_t> poly(m_lassoPts.size());
    for(size_t i = 0; i < m_lassoPts.size(); i++)
    {
        poly[i] = pt_t(m_lassoPts[i].x(), m_lassoPts[i].y());
    }
    m_flc->SetSourcePoly(&poly[0], (int)poly.size());

    qDebug("Area of boundary: %lf", m_flc->Mesh()->areaOfBoundaryPoly());

    updateGL();
}

int total_iters = 0;
void GlTestFlcWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
//    case 'S':
//        m_flc->ComputeInterpolant(2);
//        total_iters += 2;
//        qDebug("total_iters is %d", total_iters);
//        update();
//    break;
    case Qt::Key_M:
        m_flc->drawMesh(m_drawmesh = ! m_drawmesh);
        update();
    break;
    case Qt::Key_I:
        m_flc->doInterp(m_doInterp = ! m_doInterp);
        update();
    break;
    case Qt::Key_Escape:
        m_flc->Clear();
        update();
        m_interactionState = NO_SELECTION;
    break;
    }

    m_curKey = (Qt::Key)event->key();
}

void GlTestFlcWidget::keyReleaseEvent(QKeyEvent *event)
{
    m_curKey = Qt::Key_unknown;
}

void GlTestFlcWidget::mouserelease_region_selected(QMouseEvent *event)
{}

void GlTestFlcWidget::mouserelease_rotating(QMouseEvent *event)
{}

void GlTestFlcWidget::mouserelease_translating(QMouseEvent *event)
{
    updateGL();
}

void GlTestFlcWidget::mouserelease_scaling(QMouseEvent *event)
{}
