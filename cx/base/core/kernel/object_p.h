//  Copyright © 2025 <dingjing@live.cn>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
//  documentation files (the “Software”), to deal in the Software without restriction, including without limitation the
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright
//  notice and this permission notice shall be included in all copies or substantial portions of the Software. THE
//  SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
//  OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
//  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//
// Created by dingjing on 25-5-12.
//

#ifndef clibrary_OBJECT_P_H
#define clibrary_OBJECT_P_H

#include "3thrd/macros/macros.h"
#include "cx/base/core/global/global.h"
#include "cx/base/core/global/type-info.h"

CX_BEGIN_NAMESPACE

class CXList;
class CXString;
class CXObject;
class CXVector;
class CXVariant;
class CXPointer;
class CXByteArray;
class CXThreadData;
class CXObjectData;
class CXObjectPrivate;
class CXObjectUserData;
class CXObjectConnectionListVector;
namespace CXSharedPointer
{
    struct ExternalRefCountData;
}

struct CXSignalSpyCallbackSet
{
    typedef void (*BeginCallback)(CXObject *caller, int signalOrMethodIndex, void **argv);
    typedef void (*EndCallback)(CXObject *caller, int signalOrMethodIndex);
    BeginCallback signalBeginCallback, slotBeginCallback;
    EndCallback signalEndCallback, slotEndCallback;
};

void C_SYMBOL_EXPORT c_register_signal_spy_callbacks(CXSignalSpyCallbackSet *callbackSet);
extern C_SYMBOL_EXPORT CXBasicAtomicPointer<CXSignalSpyCallbackSet> gSignalSpyCallbackSet;
enum
{
    CXObjectPrivateVersion = CX_VERSION
};

class C_SYMBOL_EXPORT CXAbstractDeclarativeData
{
public:
    static void (*destroyed)(CXAbstractDeclarativeData *, CXObject *);
    static void (*destroyed_qml1)(CXAbstractDeclarativeData *, CXObject *);
    static void (*parentChanged)(CXAbstractDeclarativeData *, CXObject *, CXObject *);
    static void (*signalEmitted)(CXAbstractDeclarativeData *, CXObject *, int, void **);
    static int (*receivers)(CXAbstractDeclarativeData *, const CXObject *, int);
    static bool (*isSignalConnected)(CXAbstractDeclarativeData *, const CXObject *, int);
    static void (*setWidgetParent)(CXObject *, CXObject *);
};

struct CXAbstractDeclarativeDataImpl : public CXAbstractDeclarativeData
{
    cuint32 ownedByQml1 : 1;
    cuint32 unused : 31;
};

class C_SYMBOL_EXPORT CXObjectPrivate : public CXObjectData
{
    C_DECLARE_PUBLIC(CXObject)
public:
    struct ExtraData
    {
        ExtraData() {}
#ifndef C_NO_USERDATA
        CXVector<CXObjectUserData *> userData;
#endif
        CXList<CXByteArray> propertyNames;
        CXVector<CXVariant> propertyValues;
        CXVector<int> runningTimers;
        CXList<CXPointer<CXObject>> eventFilters;
        CXString objectName;
    };

    typedef void (*StaticMetaCallFunction)(CXObject *, CXMetaObject::Call, int, void **);
    struct Connection;
    struct SignalVector;

    struct ConnectionOrSignalVector
    {
        union
        {
            // linked list of orphaned connections that need cleaning up
            ConnectionOrSignalVector *nextInOrphanList;
            // linked list of connections connected to slots in this object
            Connection *next;
        };

        static SignalVector *asSignalVector(ConnectionOrSignalVector *c)
        {
            if (reinterpret_cast<cuintptr>(c) & 1) {
                return reinterpret_cast<SignalVector *>(reinterpret_cast<cuintptr>(c) & ~static_cast<cuintptr>(1u));
            }
            return nullptr;
        }

        static Connection *fromSignalVector(SignalVector *v)
        {
            return reinterpret_cast<Connection *>(reinterpret_cast<cuintptr>(v) | static_cast<cuintptr>(1u));
        }
    };

    struct Connection : public ConnectionOrSignalVector
    {
        // linked list of connections connected to slots in this object, next is in base class
        Connection **prev;
        // linked list of connections connected to signals in this object
        CXAtomicPointer<Connection> nextConnectionList;
        Connection *prevConnectionList;

        CXObject *sender;
        CXAtomicPointer<CXObject> receiver;
        CXAtomicPointer<CXThreadData> receiverThreadData;
        union
        {
            StaticMetaCallFunction callFunction;
            CXPrivate::CXSlotObjectBase *slotObj;
        };
        CXAtomicPointer<const int> argumentTypes;
        CXAtomicInt ref_;
        cuint id = 0;
        cushort method_offset;
        cushort method_relative;
        signed int signal_index : 27; // In signal range (see QObjectPrivate::signalIndex())
        cushort connectionType : 3; // 0 == auto, 1 == direct, 2 == queued, 4 == blocking
        cushort isSlotObject : 1;
        cushort ownArgumentTypes : 1;
        Connection() : ref_(2), ownArgumentTypes(true)
        {
            // ref_ is 2 for the use in the internal lists, and for the use in QMetaObject::Connection
        }
        ~Connection();
        int method() const
        {
            C_ASSERT(!isSlotObject);
            return method_offset + method_relative;
        }
        void ref() { ref_.ref(); }
        void freeSlotObject()
        {
            if (isSlotObject) {
                slotObj->destroyIfLastRef();
                isSlotObject = false;
            }
        }

        void deref()
        {
            if (!ref_.deref()) {
                C_ASSERT(!receiver.loadRelaxed());
                C_ASSERT(!isSlotObject);
                delete this;
            }
        }
    };

    // ConnectionList is a singly-linked list
    struct ConnectionList
    {
        CXAtomicPointer<Connection> first;
        CXAtomicPointer<Connection> last;
    };

    struct Sender
    {
        Sender(CXObject *receiver, CXObject *sender, int signal) : receiver(receiver), sender(sender), signal(signal)
        {
            if (receiver) {
                ConnectionData *cd = receiver->d_func()->connections.loadRelaxed();
                previous = cd->currentSender;
                cd->currentSender = this;
            }
        }

        ~Sender()
        {
            if (receiver) {
                receiver->d_func()->connections.loadRelaxed()->currentSender = previous;
            }
        }

        void receiverDeleted()
        {
            Sender *s = this;
            while (s) {
                s->receiver = nullptr;
                s = s->previous;
            }
        }

        Sender *previous;
        CXObject *receiver;
        CXObject *sender;
        int signal;
    };

    struct SignalVector : public ConnectionOrSignalVector
    {
        cuintptr allocated;
        // ConnectionList signals[]
        ConnectionList &at(int i) { return reinterpret_cast<ConnectionList *>(this + 1)[i + 1]; }

        const ConnectionList &at(int i) const { return reinterpret_cast<const ConnectionList *>(this + 1)[i + 1]; }

        int count() const { return static_cast<int>(allocated); }
    };

    struct ConnectionData
    {
        // the id below is used to avoid activating new connections. When the object gets
        // deleted it's set to 0, so that signal emission stops
        CXAtomicInteger<cuint> currentConnectionId;
        CXAtomicInt ref;
        CXAtomicPointer<SignalVector> signalVector;
        Connection *senders = nullptr;
        Sender *currentSender = nullptr; // object currently activating the object
        CXAtomicPointer<Connection> orphaned;

        ~ConnectionData()
        {
            C_ASSERT(ref.loadRelaxed() == 0);
            auto *c = orphaned.fetchAndStoreRelaxed(nullptr);
            if (c) {
                deleteOrphaned(c);
            }
            SignalVector *v = signalVector.loadRelaxed();
            c_free(v);
        }

        // must be called on the senders connection data
        // assumes the senders and receivers lock are held
        void removeConnection(Connection *c);
        enum LockPolicy
        {
            NeedToLock,
            // Beware that we need to temporarily release the lock
            // and thus calling code must carefully consider whether
            // invariants still hold.
            AlreadyLockedAndTemporarilyReleasingLock
        };

        void cleanOrphanedConnections(CXObject *sender, LockPolicy lockPolicy = NeedToLock)
        {
            if (orphaned.loadRelaxed() && ref.loadAcquire() == 1) {
                cleanOrphanedConnectionsImpl(sender, lockPolicy);
            }
        }

        void cleanOrphanedConnectionsImpl(CXObject *sender, LockPolicy lockPolicy);

        ConnectionList &connectionsForSignal(int signal) { return signalVector.loadRelaxed()->at(signal); }

        void resizeSignalVector(cuint size)
        {
            SignalVector *vector = this->signalVector.loadRelaxed();
            if (vector && vector->allocated > size) {
                return;
            }
            size = (size + 7) & ~7;
            SignalVector *newVector =
                reinterpret_cast<SignalVector *>(c_malloc0(sizeof(SignalVector) + (size + 1) * sizeof(ConnectionList)));
            int start = -1;
            if (vector) {
                memcpy(newVector, vector, sizeof(SignalVector) + (vector->allocated + 1) * sizeof(ConnectionList));
                start = vector->count();
            }
            for (int i = start; i < int(size); ++i) {
                newVector->at(i) = ConnectionList();
            }
            newVector->next = nullptr;
            newVector->allocated = size;

            signalVector.storeRelaxed(newVector);
            if (vector) {
                Connection *o = nullptr;
                /**
                 * No ABA issue here: When adding a node, we only care about the list head, it doesn't
                 * matter if the tail changes.
                 */
                do {
                    o = orphaned.loadRelaxed();
                    vector->nextInOrphanList = o;
                }
                while (!orphaned.testAndSetRelease(o, ConnectionOrSignalVector::fromSignalVector(vector)));
            }
        }

        int signalVectorCount() const { return signalVector.loadAcquire() ? signalVector.loadRelaxed()->count() : -1; }

        static void deleteOrphaned(ConnectionOrSignalVector *c);
    };

    CXObjectPrivate(int version = CXObjectPrivateVersion);
    virtual ~CXObjectPrivate();
    void deleteChildren();

    inline void checkForIncompatibleLibraryVersion(int version) const;

    void setParent_helper(CXObject *);
    void moveToThread_helper();
    void setThreadData_helper(CXThreadData *currentData, CXThreadData *targetData);
    void _q_reregisterTimers(void *pointer);

    bool isSender(const CXObject *receiver, const char *signal) const;
    CXObjectList receiverList(const char *signal) const;
    CXObjectList senderList() const;

    void addConnection(int signal, Connection *c);

    static CXObjectPrivate *get(CXObject *o) { return o->d_func(); }
    static const CXObjectPrivate *get(const CXObject *o) { return o->d_func(); }

    int signalIndex(const char *signalName, const CXMetaObject **meta = nullptr) const;
    bool isSignalConnected(cuint signalIdx, bool checkDeclarative = true) const;
    bool maybeSignalConnected(cuint signalIndex) const;
    inline bool isDeclarativeSignalConnected(cuint signalIdx) const;

    // To allow abitrary objects to call connectNotify()/disconnectNotify() without making
    // the API public in QObject. This is used by QQmlNotifierEndpoint.
    inline void connectNotify(const CXMetaMethod &signal);
    inline void disconnectNotify(const CXMetaMethod &signal);

    template <typename Func1, typename Func2>
    static inline CXMetaObject::Connection
    connect(const typename CXPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
            const typename CXPrivate::FunctionPointer<Func2>::Object *receiverPrivate, Func2 slot,
            CX::ConnectionType type = CX::AutoConnection);

    template <typename Func1, typename Func2>
    static inline bool disconnect(const typename CXPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
                                  const typename CXPrivate::FunctionPointer<Func2>::Object *receiverPrivate,
                                  Func2 slot);

    static CXMetaObject::Connection connectImpl(const CXObject *sender, int signal_index, const CXObject *receiver,
                                                void **slot, CXPrivate::QSlotObjectBase *slotObj,
                                                CX::ConnectionType type, const int *types,
                                                const CXMetaObject *senderMetaObject);
    static CXMetaObject::Connection connect(const CXObject *sender, int signal_index,
                                            CXPrivate::QSlotObjectBase *slotObj, CX::ConnectionType type);
    static CXMetaObject::Connection connect(const CXObject *sender, int signal_index, const CXObject *receiver,
                                            CXPrivate::QSlotObjectBase *slotObj, CX::ConnectionType type);
    static bool disconnect(const CXObject *sender, int signal_index, void **slot);
    static bool disconnect(const CXObject *sender, int signal_index, const CXObject *receiver, void **slot);

    void ensureConnectionData()
    {
        if (connections.loadRelaxed()) {
            return;
        }
        ConnectionData *cd = new ConnectionData;
        cd->ref.ref();
        connections.storeRelaxed(cd);
    }

public:
    ExtraData *extraData; // extra data set by the user
    // This atomic requires acquire/release semantics in a few places,
    // e.g. QObject::moveToThread must synchronize with QCoreApplication::postEvent,
    // because postEvent is thread-safe.
    // However, most of the code paths involving QObject are only reentrant and
    // not thread-safe, so synchronization should not be necessary there.
    CXAtomicPointer<CXThreadData> threadData; // id of the thread that owns the object

    using ConnectionDataPointer = CXExplicitlySharedDataPointer<ConnectionData>;
    CXAtomicPointer<ConnectionData> connections;

    union
    {
        CXObject *currentChildBeingDeleted; // should only be used when QObjectData::isDeletingChildren is set
        CXAbstractDeclarativeData *declarativeData; // extra data used by the declarative module
    };

    // these objects are all used to indicate that a QObject was deleted
    // plus QPointer, which keeps a separate list
    CXAtomicPointer<CXSharedPointer::ExternalRefCountData> sharedRefcount;
};

C_DECLARE_TYPEINFO(CXObjectPrivate::ConnectionList, CX_MOVABLE_TYPE);

inline void CXObjectPrivate::checkForIncompatibleLibraryVersion(int version) const
{
#if defined(CX_BUILD_INTERNAL)
    // Don't check the version parameter in internal builds.
    // This allows incompatible versions to be loaded, possibly for testing.
    Q_UNUSED(version);
#else
    if (C_UNLIKELY(version != CXObjectPrivateVersion)) {
        C_LOG_FFATAL("Cannot mix incompatible clibrary-cx library (%d.%d.%d) with this library (%d.%d.%d)",
                (version >> 16) & 0xff, (version >> 8) & 0xff, version & 0xff,
                (CXObjectPrivateVersion >> 16) & 0xff, (CXObjectPrivateVersion >> 8) & 0xff, CXObjectPrivateVersion & 0xff);
    }
#endif
}

inline bool CXObjectPrivate::isDeclarativeSignalConnected(cuint signal_index) const
{
    return !isDeletingChildren && declarativeData && CXAbstractDeclarativeData::isSignalConnected
            && CXAbstractDeclarativeData::isSignalConnected(declarativeData, q_func(), signal_index);
}

inline void CXObjectPrivate::connectNotify(const CXMetaMethod &signal)
{
    q_ptr->connectNotify(signal);
}

inline void CXObjectPrivate::disconnectNotify(const CXMetaMethod &signal)
{
    q_ptr->disconnectNotify(signal);
}

namespace CXPrivate {
template<typename Func, typename Args, typename R> class CXPrivateSlotObject : public CXSlotObjectBase
{
    typedef CXPrivate::FunctionPointer<Func> FuncType;
    Func function;
    static void impl(int which, CXSlotObjectBase *this_, CXObject *r, void **a, bool *ret)
    {
        switch (which) {
            case Destroy:
                delete static_cast<CXPrivateSlotObject*>(this_);
                break;
            case Call:
                FuncType::template call<Args, R>(static_cast<CXPrivateSlotObject*>(this_)->function,
                                                 static_cast<typename FuncType::Object *>(CXObjectPrivate::get(r)), a);
                break;
            case Compare:
                *ret = *reinterpret_cast<Func *>(a) == static_cast<CXPrivateSlotObject*>(this_)->function;
                break;
            case NumOperations: ;
        }
    }
public:
    explicit CXPrivateSlotObject(Func f) : CXSlotObjectBase(&impl), function(f) {}
};
}

template <typename Func1, typename Func2>
inline CXMetaObject::Connection CXObjectPrivate::connect(
        const typename CXPrivate::FunctionPointer<Func1>::Object *sender, Func1 signal,
        const typename CXPrivate::FunctionPointer<Func2>::Object *receiverPrivate, Func2 slot,
        CX::ConnectionType type)
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
    if (type == CX::QueuedConnection || type == CX::BlockingQueuedConnection) {
        types = CXPrivate::ConnectionTypes<typename SignalType::Arguments>::types();
    }

    return CXObject::connectImpl(sender, reinterpret_cast<void **>(&signal),
        receiverPrivate->q_ptr, reinterpret_cast<void **>(&slot),
        new CXPrivate::CXPrivateSlotObject<Func2, typename CXPrivate::List_Left<typename SignalType::Arguments, SlotType::ArgumentCount>::Value,
            typename SignalType::ReturnType>(slot), type, types, &SignalType::Object::staticMetaObject);
}

template <typename Func1, typename Func2>
bool CXObjectPrivate::disconnect(const typename CXPrivate::FunctionPointer< Func1 >::Object* sender, Func1 signal,
                                const typename CXPrivate::FunctionPointer< Func2 >::Object* receiverPrivate, Func2 slot)
{
    typedef CXPrivate::FunctionPointer<Func1> SignalType;
    typedef CXPrivate::FunctionPointer<Func2> SlotType;
    CX_STATIC_ASSERT_X(CXPrivate::HasCX_OBJECT_Macro<typename SignalType::Object>::Value,
                      "No CX_OBJECT in the class with the signal");
    //compilation error if the arguments does not match.
    CX_STATIC_ASSERT_X((CXPrivate::CheckCompatibleArguments<typename SignalType::Arguments, typename SlotType::Arguments>::value),
                      "Signal and slot arguments are not compatible.");
    return CXObject::disconnectImpl(sender, reinterpret_cast<void **>(&signal),
                          receiverPrivate->q_ptr, reinterpret_cast<void **>(&slot),
                          &SignalType::Object::staticMetaObject);
}

CX_DECLARE_TYPEINFO(CXObjectPrivate::Connection, CX_MOVABLE_TYPE);
CX_DECLARE_TYPEINFO(CXObjectPrivate::Sender, CX_MOVABLE_TYPE);

class CXSemaphore;
class C_SYMBOL_EXPORT CXAbstractMetaCallEvent : public CXEvent
{
public:
    CXAbstractMetaCallEvent(const CXObject *sender, int signalId, CXSemaphore *semaphore = nullptr)
        : CXEvent(MetaCall), signalId_(signalId), sender_(sender)
#if CX_CONFIG(thread)
        , semaphore_(semaphore)
#endif
    { C_UNUSED(semaphore); }
    ~CXAbstractMetaCallEvent();

    virtual void placeMetaCall(CXObject *object) = 0;

    inline const CXObject *sender() const { return sender_; }
    inline int signalId() const { return signalId_; }

private:
    int signalId_;
    const CXObject *sender_;
#if CX_CONFIG(thread)
    CXSemaphore *semaphore_;
#endif
};

class C_SYMBOL_EXPORT CXMetaCallEvent : public CXAbstractMetaCallEvent
{
public:
    // blocking queued with semaphore - args always owned by caller
    CXMetaCallEvent(cushort method_offset, cushort method_relative,
                   QObjectPrivate::StaticMetaCallFunction callFunction,
                   const CXObject *sender, int signalId,
                   void **args, CXSemaphore *semaphore);
    CXMetaCallEvent(CXPrivate::CXSlotObjectBase *slotObj,
                   const CXObject *sender, int signalId,
                   void **args, CXSemaphore *semaphore);

    // queued - args allocated by event, copied by caller
    CXMetaCallEvent(cushort method_offset, cushort method_relative,
                   CXObjectPrivate::StaticMetaCallFunction callFunction,
                   const CXObject *sender, int signalId,
                   int nargs);
    CXMetaCallEvent(CXPrivate::CXSlotObjectBase *slotObj,
                   const CXObject *sender, int signalId,
                   int nargs);

    ~CXMetaCallEvent() override;

    inline int id() const { return d.method_offset_ + d.method_relative_; }
    inline const void * const* args() const { return d.args_; }
    inline void ** args() { return d.args_; }
    inline const int *types() const { return reinterpret_cast<int*>(d.args_ + d.nargs_); }
    inline int *types() { return reinterpret_cast<int*>(d.args_ + d.nargs_); }

    virtual void placeMetaCall(CXObject *object) override;

private:
    inline void allocArgs();

    struct Data {
        CXPrivate::CXSlotObjectBase *slotObj_;
        void **args_;
        CXObjectPrivate::StaticMetaCallFunction callFunction_;
        int nargs_;
        cushort method_offset_;
        cushort method_relative_;
    } d;
    // preallocate enough space for three arguments
    char prealloc_[3*(sizeof(void*) + sizeof(int))];
};

class CXBoolBlocker
{
    C_DISABLE_COPY_MOVE(CXBoolBlocker)
public:
    explicit inline CXBoolBlocker(bool &b, bool value=true):block(b), reset(b){block = value;}
    inline ~CXBoolBlocker(){block = reset; }
private:
    bool &block;
    bool reset;
};

void C_SYMBOL_EXPORT deleteInEventHandler(CXObject *o);

struct CXAbstractDynamicMetaObject;
struct C_SYMBOL_EXPORT CXDynamicMetaObjectData
{
    virtual ~CXDynamicMetaObjectData();
    virtual void objectDestroyed(CXObject *) { delete this; }

    virtual CXAbstractDynamicMetaObject *toDynamicMetaObject(CXObject *) = 0;
    virtual int metaCall(CXObject *, CXMetaObject::Call, int _id, void **) = 0;
};

struct C_SYMBOL_EXPORT CXAbstractDynamicMetaObject : public CXDynamicMetaObjectData, public CXMetaObject
{
    ~CXAbstractDynamicMetaObject();

    CXAbstractDynamicMetaObject *toDynamicMetaObject(CXObject *) override { return this; }
    virtual int createProperty(const char *, const char *) { return -1; }
    int metaCall(CXObject *, CXMetaObject::Call c, int _id, void **a) override
    { return metaCall(c, _id, a); }
    virtual int metaCall(CXMetaObject::Call, int _id, void **) { return _id; } // Compat overload
};


CX_END_NAMESPACE

#endif // clibrary_OBJECT_P_H
