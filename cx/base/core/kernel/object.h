//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//  THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-5-12.
//

#ifndef clibrary_OBJECT_H
#define clibrary_OBJECT_H

#include "3thrd/macros/macros.h"
#include "cx/base/core/global/define.h"
#include "cx/base/core/global/name-space.h"

CX_BEGIN_NAMESPACE

struct CXMetaObject;
struct CXDynamicMetaObjectData;

class CXEvent;
class CXRegExp;
class CXObject;
class CXThread;
class CXWidget;
class CXVariant;
class CXTimerEvent;
class CXChildEvent;
class CXObjectPrivate;
class CXObjectUserData;
class CXAccessibleWidget;

template <class T> struct CXIntegerForSizeof: CXIntegerForSize<sizeof(T)> { };
typedef CXIntegerForSize<C_CPU_WORDSIZE>::Signed cregisterint;
typedef CXIntegerForSize<C_CPU_WORDSIZE>::Unsigned cregisteruint;
typedef CXIntegerForSizeof<void*>::Unsigned cuintptr;
typedef CXIntegerForSizeof<void*>::Signed cptrdiff;
typedef cptrdiff cintptr;

typedef CXList<CXObject*> CXObjectList;

C_SYMBOL_EXPORT void _findChildren_helper(const CXObject *parent, const CXString &name,
                                           const CXMetaObject &mo, CXList<void *> *list, cx::FindChildOptions options);
C_SYMBOL_EXPORT void _findChildren_helper(const CXObject *parent, const CXRegExp &re,
                                           const CXMetaObject &mo, CXList<void *> *list, cx::FindChildOptions options);
C_SYMBOL_EXPORT void _findChildren_helper(const CXObject *parent, const CXRegularExpression &re,
                                           const CXMetaObject &mo, QXList<void *> *list, cx::FindChildOptions options);
C_SYMBOL_EXPORT CXObject* _findChild_helper(const CXObject *parent, const CXString &name, const CXMetaObject &mo, cx::FindChildOptions options);

class C_SYMBOL_EXPORT CXObjectData
{
    C_DISABLE_COPY(CXObjectData)
public:
    CXObjectData() = default;
    virtual ~CXObjectData() = 0;
    CXObject *q_ptr;
    CXObject *parent;
    CXObjectList children;

    cuint isWidget : 1;
    cuint blockSig : 1;
    cuint wasDeleted : 1;
    cuint isDeletingChildren : 1;
    cuint sendChildEvents : 1;
    cuint receiveChildEvents : 1;
    cuint isWindow : 1; //for QWindow
    cuint deleteLaterCalled : 1;
    cuint unused : 24;
    cint postedEvents;
    CXDynamicMetaObjectData *metaObject;
    CXMetaObject *dynamicMetaObject() const;

#ifdef CX_DEBUG
    enum { CheckForParentChildLoopsWarnDepth = 4096 };
#endif
};


class C_SYMBOL_EXPORT CXObject
{
    CX_OBJECT
    CX_PROPERTY(CXString objectName READ objectName WRITE setObjectName NOTIFY objectNameChanged)
    C_DECLARE_PRIVATE(CXObject)

public:
    CX_INVOKABLE explicit CXObject(CXObject *parent=nullptr);
    virtual ~CXObject();

    virtual bool event(CXEvent *event);
    virtual bool eventFilter(CXObject *watched, CXEvent *event);

    static CXString tr(const char *sourceText, const char * = nullptr, int = -1)
        { return CXString::fromUtf8(sourceText); }
    C_DEPRECATED static CXString trUtf8(const char *sourceText, const char * = nullptr, int = -1)
        { return CXString::fromUtf8(sourceText); }

    CXString objectName() const;
    void setObjectName(const CXString &name);

    inline bool isWidgetType() const { return d_ptr->isWidget; }
    inline bool isWindowType() const { return d_ptr->isWindow; }

    inline bool signalsBlocked() const noexcept { return d_ptr->blockSig; }
    bool blockSignals(bool b) noexcept;

    CXThread *thread() const;
    void moveToThread(CXThread *thread);

    int startTimer(int interval, cx::TimerType timerType = cx::CoarseTimer);
#if __has_include(<chrono>)
    inline int startTimer(std::chrono::milliseconds time, cx::TimerType timerType = cx::CoarseTimer)
    {
        return startTimer(int(time.count()), timerType);
    }
#endif
    void killTimer(int id);

    template<typename T>
    inline T findChild(const CXString &aName = CXString(), cx::FindChildOptions options = cx::FindChildrenRecursively) const
    {
        typedef typename std::remove_cv<typename std::remove_pointer<T>::type>::type ObjType;
        return static_cast<T>(_findChild_helper(this, aName, ObjType::staticMetaObject, options));
    }

    template<typename T>
    inline CXList<T> findChildren(const CXString &aName = CXString(), cx::FindChildOptions options = cx::FindChildrenRecursively) const
    {
        typedef typename std::remove_cv<typename std::remove_pointer<T>::type>::type ObjType;
        CXList<T> list;
        _findChildren_helper(this, aName, ObjType::staticMetaObject, reinterpret_cast<CXList<void *> *>(&list), options);
        return list;
    }

    template<typename T>
    inline CXList<T> findChildren(const CXRegExp &re, cx::FindChildOptions options = cx::FindChildrenRecursively) const
    {
        typedef typename std::remove_cv<typename std::remove_pointer<T>::type>::type ObjType;
        CXList<T> list;
        _findChildren_helper(this, re, ObjType::staticMetaObject,
                                reinterpret_cast<CXList<void *> *>(&list), options);
        return list;
    }

    template<typename T>
    inline CXList<T> findChildren(const CXRegularExpression &re, cx::FindChildOptions options = cx::FindChildrenRecursively) const
    {
        typedef typename std::remove_cv<typename std::remove_pointer<T>::type>::type ObjType;
        CXList<T> list;
        _findChildren_helper(this, re, ObjType::staticMetaObject,
                                reinterpret_cast<CXList<void *> *>(&list), options);
        return list;
    }

    inline const CXObjectList &children() const { return d_ptr->children; }

    void setParent(CXObject *parent);
    void installEventFilter(CXObject *filterObj);
    void removeEventFilter(CXObject *obj);

    static CXMetaObject::Connection connect(const CXObject *sender, const char *signal,
                        const CXObject *receiver, const char *member, cx::ConnectionType = cx::AutoConnection);

    static CXMetaObject::Connection connect(const CXObject *sender, const CXMetaMethod &signal,
                        const CXObject *receiver, const CXMetaMethod &method,
                        cx::ConnectionType type = cx::AutoConnection);

    inline CXMetaObject::Connection connect(const CXObject *sender, const char *signal,
                        const char *member, cx::ConnectionType type = cx::AutoConnection) const;

    //Connect a signal to a pointer to qobject member function
    template <typename Func1, typename Func2>
    static inline CXMetaObject::Connection connect(const typename CXPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
                                     const typename CXPrivate::FunctionPointer<Func2>::Object *receiver, Func2 slot,
                                     cx::ConnectionType type = cx::AutoConnection)
    {
        typedef CXPrivate::FunctionPointer<Func1> SignalType;
        typedef CXPrivate::FunctionPointer<Func2> SlotType;

        CX_STATIC_ASSERT_X(CXPrivate::HasQ_OBJECT_Macro<typename SignalType::Object>::Value,
                          "No CX_OBJECT in the class with the signal");

        //compilation error if the arguments does not match.
        CX_STATIC_ASSERT_X(int(SignalType::ArgumentCount) >= int(SlotType::ArgumentCount),
                          "The slot requires more arguments than the signal provides.");
        CX_STATIC_ASSERT_X((CXPrivate::CheckCompatibleArguments<typename SignalType::Arguments, typename SlotType::Arguments>::value),
                          "Signal and slot arguments are not compatible.");
        CX_STATIC_ASSERT_X((CXPrivate::AreArgumentsCompatible<typename SlotType::ReturnType, typename SignalType::ReturnType>::value),
                          "Return type of the slot is not compatible with the return type of the signal.");

        const int *types = nullptr;
        if (type == cx::QueuedConnection || type == cx::BlockingQueuedConnection)
            types = CXPrivate::ConnectionTypes<typename SignalType::Arguments>::types();

        return connectImpl(sender, reinterpret_cast<void **>(&signal),
                           receiver, reinterpret_cast<void **>(&slot),
                           new CXPrivate::QSlotObject<Func2, typename CXPrivate::List_Left<typename SignalType::Arguments,
                           SlotType::ArgumentCount>::Value, typename SignalType::ReturnType>(slot),
                            type, types, &SignalType::Object::staticMetaObject);
    }

    //connect to a function pointer  (not a member)
    template <typename Func1, typename Func2>
    static inline typename std::enable_if<int(CXPrivate::FunctionPointer<Func2>::ArgumentCount) >= 0, CXMetaObject::Connection>::type
            connect(const typename CXPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal, Func2 slot)
    {
        return connect(sender, signal, sender, slot, cx::DirectConnection);
    }

    //connect to a function pointer  (not a member)
    template <typename Func1, typename Func2>
    static inline typename std::enable_if<int(CXPrivate::FunctionPointer<Func2>::ArgumentCount) >= 0 &&
                                          !CXPrivate::FunctionPointer<Func2>::IsPointerToMemberFunction, CXMetaObject::Connection>::type
            connect(const typename CXPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal, const CXObject *context, Func2 slot,
                    cx::ConnectionType type = cx::AutoConnection)
    {
        typedef CXPrivate::FunctionPointer<Func1> SignalType;
        typedef CXPrivate::FunctionPointer<Func2> SlotType;

        CX_STATIC_ASSERT_X(CXPrivate::HasQ_OBJECT_Macro<typename SignalType::Object>::Value,
                          "No CX_OBJECT in the class with the signal");

        //compilation error if the arguments does not match.
        CX_STATIC_ASSERT_X(int(SignalType::ArgumentCount) >= int(SlotType::ArgumentCount),
                          "The slot requires more arguments than the signal provides.");
        CX_STATIC_ASSERT_X((CXPrivate::CheckCompatibleArguments<typename SignalType::Arguments, typename SlotType::Arguments>::value),
                          "Signal and slot arguments are not compatible.");
        CX_STATIC_ASSERT_X((CXPrivate::AreArgumentsCompatible<typename SlotType::ReturnType, typename SignalType::ReturnType>::value),
                          "Return type of the slot is not compatible with the return type of the signal.");

        const int *types = nullptr;
        if (type == cx::QueuedConnection || type == cx::BlockingQueuedConnection)
            types = CXPrivate::ConnectionTypes<typename SignalType::Arguments>::types();

        return connectImpl(sender, reinterpret_cast<void **>(&signal), context, nullptr,
                           new CXPrivate::QStaticSlotObject<Func2,
                                                 typename CXPrivate::List_Left<typename SignalType::Arguments, SlotType::ArgumentCount>::Value,
                                                 typename SignalType::ReturnType>(slot),
                           type, types, &SignalType::Object::staticMetaObject);
    }

    //connect to a functor
    template <typename Func1, typename Func2>
    static inline typename std::enable_if<CXPrivate::FunctionPointer<Func2>::ArgumentCount == -1, CXMetaObject::Connection>::type
            connect(const typename CXPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal, Func2 slot)
    {
        return connect(sender, signal, sender, std::move(slot), cx::DirectConnection);
    }

    //connect to a functor, with a "context" object defining in which event loop is going to be executed
    template <typename Func1, typename Func2>
    static inline typename std::enable_if<CXPrivate::FunctionPointer<Func2>::ArgumentCount == -1, CXMetaObject::Connection>::type
            connect(const typename CXPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal, const CXObject *context, Func2 slot,
                    cx::ConnectionType type = cx::AutoConnection)
    {
        typedef CXPrivate::FunctionPointer<Func1> SignalType;
        const int FunctorArgumentCount = CXPrivate::ComputeFunctorArgumentCount<Func2 , typename SignalType::Arguments>::Value;

        CX_STATIC_ASSERT_X((FunctorArgumentCount >= 0),
                          "Signal and slot arguments are not compatible.");
        const int SlotArgumentCount = (FunctorArgumentCount >= 0) ? FunctorArgumentCount : 0;
        typedef typename CXPrivate::FunctorReturnType<Func2, typename CXPrivate::List_Left<typename SignalType::Arguments, SlotArgumentCount>::Value>::Value SlotReturnType;

        CX_STATIC_ASSERT_X((CXPrivate::AreArgumentsCompatible<SlotReturnType, typename SignalType::ReturnType>::value),
                          "Return type of the slot is not compatible with the return type of the signal.");

        CX_STATIC_ASSERT_X(CXPrivate::HasQ_OBJECT_Macro<typename SignalType::Object>::Value,
                          "No Q_OBJECT in the class with the signal");

        const int *types = nullptr;
        if (type == cx::QueuedConnection || type == cx::BlockingQueuedConnection)
            types = CXPrivate::ConnectionTypes<typename SignalType::Arguments>::types();

        return connectImpl(sender, reinterpret_cast<void **>(&signal), context, nullptr,
                           new CXPrivate::QFunctorSlotObject<Func2, SlotArgumentCount,
                                typename CXPrivate::List_Left<typename SignalType::Arguments, SlotArgumentCount>::Value,
                                typename SignalType::ReturnType>(std::move(slot)),
                           type, types, &SignalType::Object::staticMetaObject);
    }

    static bool disconnect(const CXObject *sender, const char *signal,
                           const CXObject *receiver, const char *member);
    static bool disconnect(const CXObject *sender, const CXMetaMethod &signal,
                           const CXObject *receiver, const CXMetaMethod &member);
    inline bool disconnect(const char *signal = nullptr,
                           const CXObject *receiver = nullptr, const char *member = nullptr) const
        { return disconnect(this, signal, receiver, member); }
    inline bool disconnect(const CXObject *receiver, const char *member = nullptr) const
        { return disconnect(this, nullptr, receiver, member); }
    static bool disconnect(const CXMetaObject::Connection &);

#ifdef Q_CLANG_QDOC
    template<typename PointerToMemberFunction>
    static bool disconnect(const CXObject *sender, PointerToMemberFunction signal, const CXObject *receiver, PointerToMemberFunction method);
#else
    template <typename Func1, typename Func2>
    static inline bool disconnect(const typename CXPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
                                  const typename CXPrivate::FunctionPointer<Func2>::Object *receiver, Func2 slot)
    {
        typedef CXPrivate::FunctionPointer<Func1> SignalType;
        typedef CXPrivate::FunctionPointer<Func2> SlotType;

        CX_STATIC_ASSERT_X(CXPrivate::HasQ_OBJECT_Macro<typename SignalType::Object>::Value,
                          "No Q_OBJECT in the class with the signal");

        //compilation error if the arguments does not match.
        CX_STATIC_ASSERT_X((CXPrivate::CheckCompatibleArguments<typename SignalType::Arguments, typename SlotType::Arguments>::value),
                          "Signal and slot arguments are not compatible.");

        return disconnectImpl(sender, reinterpret_cast<void **>(&signal), receiver, reinterpret_cast<void **>(&slot),
                              &SignalType::Object::staticMetaObject);
    }
    template <typename Func1>
    static inline bool disconnect(const typename CXPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
                                  const CXObject *receiver, void **zero)
    {
        // This is the overload for when one wish to disconnect a signal from any slot. (slot=nullptr)
        // Since the function template parameter cannot be deduced from '0', we use a
        // dummy void ** parameter that must be equal to 0
        CX_ASSERT(!zero);
        typedef CXPrivate::FunctionPointer<Func1> SignalType;
        return disconnectImpl(sender, reinterpret_cast<void **>(&signal), receiver, zero,
                              &SignalType::Object::staticMetaObject);
    }
#endif //Q_CLANG_QDOC


#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void dumpObjectTree(); // ### Qt 6: remove
    void dumpObjectInfo(); // ### Qt 6: remove
#endif
    void dumpObjectTree() const;
    void dumpObjectInfo() const;

#ifndef QT_NO_PROPERTIES
    bool setProperty(const char *name, const CXVariant &value);
    CXVariant property(const char *name) const;
    CXList<CXByteArray> dynamicPropertyNames() const;
#endif // QT_NO_PROPERTIES

    static cuint registerUserData();
    void setUserData(cuint id, CXObjectUserData* data);
    CXObjectUserData* userData(cuint id) const;

CX_SIGNALS:
    void destroyed(CXObject * = nullptr);
    void objectNameChanged(const CXString &objectName, CXPrivateSignal);

public:
    inline CXObject *parent() const { return d_ptr->parent; }

    inline bool inherits(const char *classname) const
        { return const_cast<CXObject *>(this)->cx_metacast(classname) != nullptr; }

public CX_SLOTS:
    void deleteLater();

protected:
    CXObject *sender() const;
    int senderSignalIndex() const;
    int receivers(const char* signal) const;
    bool isSignalConnected(const CXMetaMethod &signal) const;

    virtual void timerEvent(CXTimerEvent *event);
    virtual void childEvent(CXChildEvent *event);
    virtual void customEvent(CXEvent *event);

    virtual void connectNotify(const CXMetaMethod &signal);
    virtual void disconnectNotify(const CXMetaMethod &signal);

protected:
    CXObject(CXObjectPrivate &dd, CXObject *parent = nullptr);

protected:
    CXScopedPointer<CXObjectData> d_ptr;

    static const CXMetaObject staticQtMetaObject;
    friend inline const CXMetaObject *qt_getQtMetaObject() noexcept;

    friend struct CXMetaObject;
    friend struct CXMetaObjectPrivate;
    friend class CXMetaCallEvent;
    friend class CXApplication;
    friend class CXApplicationPrivate;
    friend class CXCoreApplication;
    friend class CXCoreApplicationPrivate;
    friend class CXWidget;
    friend class CXAccessibleWidget;
    friend class CXThreadData;

private:
    C_DISABLE_COPY(CXObject)
    C_PRIVATE_SLOT(d_func(), void _q_reregisterTimers(void *))

private:
    static CXMetaObject::Connection connectImpl(const CXObject *sender, void **signal,
                                               const CXObject *receiver, void **slotPtr,
                                               CXPrivate::QSlotObjectBase *slot, cx::ConnectionType type,
                                               const int *types, const CXMetaObject *senderMetaObject);

    static bool disconnectImpl(const CXObject *sender, void **signal, const CXObject *receiver, void **slot,
                               const CXMetaObject *senderMetaObject);

};

inline CXMetaObject::Connection CXObject::connect(const CXObject *asender, const char *asignal,
                                            const char *amember, cx::ConnectionType atype) const
{ return connect(asender, asignal, this, amember, atype); }

inline const CXMetaObject *qt_getQtMetaObject() noexcept
{ return &CXObject::staticQtMetaObject; }

class C_SYMBOL_EXPORT CXObjectUserData
{
    C_DISABLE_COPY(CXObjectUserData)
public:
    CXObjectUserData() = default;
    virtual ~CXObjectUserData();
};

template<typename T>
inline T findChild(const CXObject *o, const CXString &name = CXString())
{ return o->findChild<T>(name); }

template<typename T>
inline CXList<T> findChildren(const CXObject *o, const CXString &name = CXString())
{
    return o->findChildren<T>(name);
}

template<typename T>
inline CXList<T> findChildren(const CXObject *o, const CXRegExp &re)
{
    return o->findChildren<T>(re);
}

template <class T>
inline T cx_object_cast(CXObject *object)
{
    typedef typename std::remove_cv<typename std::remove_pointer<T>::type>::type ObjType;
    CX_STATIC_ASSERT_X(CXPrivate::HasQ_OBJECT_Macro<ObjType>::Value,
                    "cx_object_cast requires the type to have a CX_OBJECT macro");
    return static_cast<T>(ObjType::staticMetaObject.cast(object));
}

template <class T>
inline T cx_object_cast(const CXObject *object)
{
    typedef typename std::remove_cv<typename std::remove_pointer<T>::type>::type ObjType;
    CX_STATIC_ASSERT_X(CXPrivate::HasQ_OBJECT_Macro<ObjType>::Value,
                      "cx_object_cast requires the type to have a CX_OBJECT macro");
    return static_cast<T>(ObjType::staticMetaObject.cast(object));
}


template <class T> inline const char * cx_object_interface_iid()
{ return nullptr; }


#  define Q_DECLARE_INTERFACE(IFace, IId) \
    template <> inline const char *cx_object_interface_iid<IFace *>() \
    { return IId; } \
    template <> inline IFace *cx_object_cast<IFace *>(CXObject *object) \
    { return reinterpret_cast<IFace *>((object ? object->cx_metacast(IId) : nullptr)); } \
    template <> inline IFace *cx_object_cast<IFace *>(const CXObject *object) \
    { return reinterpret_cast<IFace *>((object ? const_cast<CXObject *>(object)->cx_metacast(IId) : nullptr)); }

C_SYMBOL_EXPORT CXDebug operator<<(CXDebug, const CXObject *);

class CXSignalBlocker
{
public:
    inline explicit CXSignalBlocker(CXObject *o) noexcept;
    inline explicit CXSignalBlocker(CXObject &o) noexcept;
    inline ~CXSignalBlocker();

    inline CXSignalBlocker(CXSignalBlocker &&other) noexcept;
    inline CXSignalBlocker &operator=(CXSignalBlocker &&other) noexcept;

    inline void reblock() noexcept;
    inline void unblock() noexcept;
private:
    C_DISABLE_COPY(CXSignalBlocker)
    CXObject * m_o;
    bool m_blocked;
    bool m_inhibited;
};

CXSignalBlocker::CXSignalBlocker(CXObject *o) noexcept
    : m_o(o),
      m_blocked(o && o->blockSignals(true)),
      m_inhibited(false)
{}

CXSignalBlocker::CXSignalBlocker(CXObject &o) noexcept
    : m_o(&o),
      m_blocked(o.blockSignals(true)),
      m_inhibited(false)
{}

CXSignalBlocker::CXSignalBlocker(CXSignalBlocker &&other) noexcept
    : m_o(other.m_o),
      m_blocked(other.m_blocked),
      m_inhibited(other.m_inhibited)
{
    other.m_o = nullptr;
}

CXSignalBlocker &CXSignalBlocker::operator=(CXSignalBlocker &&other) noexcept
{
    if (this != &other) {
        // if both *this and other block the same object's signals:
        // unblock *this iff our dtor would unblock, but other's wouldn't
        if (m_o != other.m_o || (!m_inhibited && other.m_inhibited)) {
            unblock();
        }
        m_o = other.m_o;
        m_blocked = other.m_blocked;
        m_inhibited = other.m_inhibited;
        // disable other:
        other.m_o = nullptr;
    }
    return *this;
}

CXSignalBlocker::~CXSignalBlocker()
{
    if (m_o && !m_inhibited) {
        m_o->blockSignals(m_blocked);
    }
}

void CXSignalBlocker::reblock() noexcept
{
    if (m_o) {
        m_o->blockSignals(true);
    }
    m_inhibited = false;
}

void CXSignalBlocker::unblock() noexcept
{
    if (m_o) {
        m_o->blockSignals(m_blocked);
    }
    m_inhibited = true;
}

namespace CXPrivate
{
    inline CXObject & deref_for_methodcall(CXObject &o) { return  o; }
    inline CXObject & deref_for_methodcall(CXObject *o) { return *o; }
}
#define CX_SET_OBJECT_NAME(obj) CX_PREPEND_NAMESPACE(CXPrivate)::deref_for_methodcall(obj).setObjectName(CXLatin1String(#obj))


CX_END_NAMESPACE

#endif // clibrary_OBJECT_H
