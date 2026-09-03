#include <QDebug>
#include "evaluator.h"

// 初始化 NDTEvaluator 对象。设置 QTimer 用于定期评估，并将其超时信号连接到 Evaluate() 槽函数。
NDTEvaluator::NDTEvaluator(QObject *parent)
    : QObject(parent)
{
    _timer = new QTimer(this); // 创建一个新的 QTimer 对象，并将其父对象设置为当前对象。
    connect(_timer, SIGNAL(timeout()), this, SLOT(Evaluate())); // 将 QTimer 的 timeout() 信号连接到本对象的 Evaluate() 槽函数。
}
// NDTEvaluator 类的析构函数
NDTEvaluator::~NDTEvaluator()
{
    if (_timer)
        delete _timer; // 如果 _timer 对象存在，则删除它。
}
// 线程安全地增加给定标签的计数器。
void NDTEvaluator::Hit(const QString& tag)
{
    _hit_lock.lock(); // 使用互斥锁保护并发访问 _counters。
    if (_counters.contains(tag))
        _counters[tag]++; // 如果 _counters 中已经包含了标签 tag，则增加其对应的计数器。
    else
        _counters[tag] = 1; // 首次命中，计数器从1开始
    _hit_lock.unlock(); // 解锁互斥锁，允许其他线程访问 _counters。
}

// 启动定时器以开始定期评估。
void NDTEvaluator::Run()
{
    qDebug() << "NDTEvaluator Run"; // 输出调试信息，表示 NDTEvaluator 开始运行。
    if (_timer)
        _timer->start(1000); // 启动定时器，设置定时间隔为 1000 毫秒（即 1 秒）。
}
// 停止定时器以暂停定期评估。
void NDTEvaluator::Stop()
{
    qDebug() << "NDTEvaluator Stop"; // 输出调试信息，表示 NDTEvaluator 停止运行。
    if (_timer)
        _timer->stop(); // 停止定时器。
}
// 评估存储在 _counters 中的计数器，构造信息字符串并在评估后清空计数器。
void NDTEvaluator::Evaluate()
{
    _hit_lock.lock(); // 使用互斥锁保护并发访问 _counters。
    QHashIterator<QString, int> i(_counters); // 创建一个 QHash 迭代器，用于遍历 _counters。
    while (i.hasNext()) {
        i.next();
        _info << "device " + i.key() + " " + QString::number(i.value()) + " " + "raw_data packages per second."; // 构造信息字符串，包含设备名、计数器值以及描述信息。
    }
    _counters.clear(); // 清空 _counters 中的所有条目，准备下一轮计数。
    _hit_lock.unlock(); // 解锁互斥锁，允许其他线程访问 _counters。
}
// 返回在评估过程中收集的信息字符串列表。
QStringList NDTEvaluator::getInfo()
{
    return _info; // 返回保存了评估信息的 QStringList 对象。
}
// 清空信息字符串列表。
void NDTEvaluator::clearInfo()
{
    _info.clear(); // 清空 _info QStringList 对象。
}
