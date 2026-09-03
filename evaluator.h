#ifndef _EVALUATOR_H
#define _EVALUATOR_H
#include <QVector>
#include <QHash>
#include <QString>
#include <QMutex>
#include <QTimer>
//-----------------------------------------------------------------------------
//
class NDTEvaluator :  public QObject
{
    Q_OBJECT
        
public:
    NDTEvaluator(QObject *parent);
    ~NDTEvaluator();
    // 线程安全地增加给定标签的计数器。
    void Hit(const QString& tag);
    // 启动定时器
    void Run();
    // 停止定时器
    void Stop();
    // 返回在评估过程中收集的信息字符串列表。
    QStringList getInfo();
    // 清空信息字符串列表。
    void clearInfo();

protected:
    QStringList _info;
    QTimer *_timer;
	QMutex _hit_lock;    
    QHash<QString, int> _counters;

private slots:
    void Evaluate();
};

#endif
