#ifndef GLTESTFLCWIDGET_H
#define GLTESTFLCWIDGET_H


#include <QGLWidget>
#include <vector>

class FastLaplaceComper;

class MainWindow;

class GlTestFlcWidget : public QGLWidget
{
    friend class MainWindow;

public:

    GlTestFlcWidget(QWidget *parent = NULL);


    enum InteractionState
    {
        NO_SELECTION,
        SELECTING,
        REGION_SELECTED,
        ROTATING,
        TRANSLATING,
        SCALING
    };


public slots:


protected:

    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void showEvent(QShowEvent *event);

    void init_lights();
    float m_rotate_x;
    float m_rotate_y;
    QPoint m_drag_offset, m_drag_start;
    bool m_drawmesh;
    bool m_doInterp;
    QPoint m_last_pos;
    QPoint m_selectlet1, m_selectlet2;

    FastLaplaceComper *m_flc;

    InteractionState m_interactionState;

    std::vector<QPoint> m_lassoPts;

    Qt::Key m_curKey;

    void mousepress_no_selection(QMouseEvent*);
    void mousepress_region_selected(QMouseEvent*);
    void mousemove_selecting(QMouseEvent*);
    void mousemove_region_selected(QMouseEvent*);
    void mousemove_rotating(QMouseEvent*);
    void mousemove_scaling(QMouseEvent*);
    void mousemove_translating(QMouseEvent*);
    void mouserelease_selecting(QMouseEvent *event);
    void mouserelease_region_selected(QMouseEvent *event);
    void mouserelease_rotating(QMouseEvent *event);
    void mouserelease_translating(QMouseEvent *event);
    void mouserelease_scaling(QMouseEvent *event);
};

#endif // GLTESTFLCWIDGET_H
