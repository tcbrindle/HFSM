#pragma once

#include "detail/array.hpp"
#include "detail/hash_table.hpp"
#include "detail/type_info.hpp"

#include <limits>

namespace hfsm {

////////////////////////////////////////////////////////////////////////////////

template <typename TContext = void, typename TTime = float, unsigned TMaxSubstitutions = 4>
class Machine {

	using TypeInfo = detail::TypeInfo;

	template <typename T, unsigned TCapacity>
	using Array = detail::Array<T, TCapacity>;

	template <typename T>
	using ArrayView = detail::ArrayView<T>;

	template <typename TKey, typename TValue, unsigned TCapacity, typename THasher = std::hash<TKey>>
	using HashTable = detail::HashTable<TKey, TValue, TCapacity, THasher>;

	//----------------------------------------------------------------------

#pragma region Utility

	using Context = TContext;
	using Time = TTime;

	using Index = unsigned char;
	enum : Index { INVALID_INDEX = std::numeric_limits<Index>::max() };

	enum { MaxSubstitutions = TMaxSubstitutions };

	//----------------------------------------------------------------------

	struct Parent {
		#pragma pack(push, 1)
			Index fork  = INVALID_INDEX;
			Index prong = INVALID_INDEX;
		#pragma pack(pop)

		HSFM_DEBUG_ONLY(TypeInfo forkType);
		HSFM_DEBUG_ONLY(TypeInfo prongType);

		inline Parent() = default;

		inline Parent(const Index fork_,
					  const Index prong_,
					  const TypeInfo HSFM_DEBUG_ONLY(forkType_),
					  const TypeInfo HSFM_DEBUG_ONLY(prongType_))
			: fork(fork_)
			, prong(prong_)
		#ifdef _DEBUG
			, forkType(forkType_)
			, prongType(prongType_)
		#endif
		{}

		inline explicit operator bool() const { return fork != INVALID_INDEX && prong != INVALID_INDEX; }
	};

	using Parents = ArrayView<Parent>;

	//----------------------------------------------------------------------

	struct StateRegistry {
	public:
		template <typename T>
		unsigned add();

	protected:
		virtual unsigned add(const TypeInfo stateType) = 0;
	};

	//······································································

	template <unsigned TCapacity>
	class StateRegistryT
		: public StateRegistry
	{
		enum { Capacity = TCapacity };

		using TypeToIndex = HashTable<TypeInfo::Native, unsigned, Capacity>;

	public:
		inline unsigned operator[] (const TypeInfo stateType) const { return *_typeToIndex.find(*stateType); }

	protected:
		virtual unsigned add(const TypeInfo stateType) override;

	private:
		TypeToIndex _typeToIndex;
	};

	//----------------------------------------------------------------------

	struct Fork {
		#pragma pack(push, 1)
			Index self		= INVALID_INDEX;
			Index active	= INVALID_INDEX;
			Index resumable = INVALID_INDEX;
			Index requested = INVALID_INDEX;
		#pragma pack(pop)

		HSFM_DEBUG_ONLY(const TypeInfo type);
		HSFM_DEBUG_ONLY(TypeInfo activeType);
		HSFM_DEBUG_ONLY(TypeInfo resumableType);
		HSFM_DEBUG_ONLY(TypeInfo requestedType);

		Fork(const Index index, const TypeInfo type_);
	};
	using ForkPointers = ArrayView<Fork*>;

	//����������������������������������������������������������������������

	template <typename T>
	struct ForkT
		: Fork
	{
		ForkT(const Index index)
			: Fork(index, TypeInfo::get<T>())
		{}
	};

	//----------------------------------------------------------------------

	struct Transition {
		enum Type {
			Restart,
			Resume,
			Schedule,

			COUNT
		};

		Type type = Restart;
		TypeInfo stateType;

		inline Transition() = default;

		inline Transition(const Type type_, const TypeInfo stateType_)
			: type(type_)
			, stateType(stateType_)
		{
			assert(type_ < Type::COUNT);
		}
	};
	using TransitionQueue = ArrayView<Transition>;

	//----------------------------------------------------------------------

	class Control;

	class TrackedTimed;

	// shortened class names
	template <typename T>
	class _S;

	template <typename, typename...>
	class _C;

	template <typename, typename...>
	class _O;

	template <typename>
	class _R;

#pragma endregion

	//----------------------------------------------------------------------

#pragma region State
public:

	class Bare {
		template <typename...>
		friend class _B;

	protected:
		using Control	 = Control;
		using Context	 = Context;
		using Time		 = Time;
		using Transition = Transition;

	//private:
		inline void preSubstitute(Context&, const Time) const {}
		inline void preEnter(Context&, const Time)  {}
		inline void preUpdate(Context&, const Time) {}
		inline void preTransition(Context&, const Time) const {}
		inline void postLeave(Context&, const Time) {}
	};

	//����������������������������������������������������������������������
private:

	class Tracked
		: public Bare
	{
		template <typename...>
		friend class _B;

	public:
		inline unsigned entryCount() const				{ return _entryCount;			 }
		inline unsigned updateCountAfterEntry() const	{ return _updateCountAfterEntry; }
		inline unsigned totalUpdateCount() const		{ return _totalUpdateCount;		 }

	//private:
		inline void preEnter(Context&, const Time)  {
			++_entryCount;
			_updateCountAfterEntry = 0;
		}

		inline void preUpdate(Context&, const Time) {
			++_updateCountAfterEntry;
			++_totalUpdateCount;
		}

	private:
		unsigned _entryCount = 0;
		unsigned _updateCountAfterEntry = 0;
		unsigned _totalUpdateCount = 0;
	};

	//����������������������������������������������������������������������

	class Timed
		: public Bare
	{
		template <typename...>
		friend class _B;

	public:
		inline auto entryTime() const { return _entryTime; }

	//private:
		inline void preEnter(Context&, const Time time) { _entryTime = time; }

	private:
		Time _entryTime;
	};

	//����������������������������������������������������������������������

	template <typename...>
	class _B;

	// � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � �

	template <typename TInjection, typename... TRest>
	class _B<TInjection, TRest...>
		: public TInjection
		, public _B<TRest...>
	{
		template <typename...>
		friend class _B;

		template <typename T>
		friend class _S;

	public:
		inline void widePreSubstitute(Context& _, const Time time) const;
		inline void widePreEnter(Context& _, const Time time);
		inline void widePreUpdate(Context& _, const Time time);
		inline void widePreTransition(Context& _, const Time time) const;
		inline void widePostLeave(Context& _, const Time time);
	};

	 // � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � � �

	template <typename TInjection>
	class _B<TInjection>
		: public TInjection
	{
		template <typename...>
		friend class _B;

		template <typename T>
		friend class _S;

	public:
		inline void substitute(Control&, Context&, const Time) const {}
		inline void enter(Context&, const Time) {}
		inline void update(Context&, const Time) {}
		inline void transition(Control&, Context&, const Time) const {}
		inline void leave(Context&, const Time) {}

	//private:
		inline void widePreSubstitute(Context& _, const Time time) const;
		inline void widePreEnter(Context& _, const Time time);
		inline void widePreUpdate(Context& _, const Time time);
		inline void widePreTransition(Context& _, const Time time) const;
		inline void widePostLeave(Context& _, const Time time);
	};

	//----------------------------------------------------------------------

	template <typename T>
	class _S {
		template <typename, typename...>
		friend class _C;

		template <typename, typename...>
		friend class _O;

		template <typename>
		friend class _R;

	public:
		using Client = T;

		enum {
			ReverseDepth = 1,
			DeepWidth	 = 0,
			StateCount	 = 1,
			ForkCount	 = 0,
			ProngCount	 = 0,
		};

	//private:
		_S(ForkPointers& /*forkPointers*/)												{}

		inline void deepLink(StateRegistry& stateRegistry,
							 const Parent parent,
							 Parents& stateParents,
							 Parents& forkParents);

		inline void deepEnterInitial(Context& context, const Time time);

		inline void deepForwardRequest(const enum Transition::Type /*type*/)			{}
		inline void deepRequestRestart()												{}
		inline void deepRequestResume()													{}

		inline void deepForwardSubstitute(Control& /*control*/,
										  Context& /*context*/,
										  const Time /*time*/) const					{}

		inline bool deepSubstitute(Control& control,
								   Context& context,
								   const Time time) const;

		inline void deepChangeToRequested(Context& /*context*/, const Time /*time*/)	{}
		inline void deepEnter(Context& context, const Time time);
		inline void deepLeave(Context& context, const Time time);

		inline bool deepUpdateAndTransition(Control& control, Context& context, const Time time);
		inline void deepUpdate(Context& context, const Time time);

	private:
		Client _client;

		HSFM_DEBUG_ONLY(const TypeInfo _type = TypeInfo::get<T>());
	};

#pragma endregion

	//----------------------------------------------------------------------

#pragma region Composite

	template <typename T, typename... TS>
	class _C final
		: _S<T>
		, public ForkT<T>
	{
		template <typename, typename...>
		friend class _C;

		template <typename, typename...>
		friend class _O;

		template <typename>
		friend class _R;
    public:
		using State = _S<T>;
		using Client = typename State::Client;

		//��������������������������������������������������������������

#pragma region Substate

		template <unsigned TN, typename...>
		struct Sub;

		//��������������������������������������������������������������

		template <unsigned TN, typename TI, typename... TR>
		struct Sub<TN, TI, TR...>
			: Sub<TN + 1, TR...>
		{
			using Initial = TI;
			using Remaining = Sub<TN + 1, TR...>;

			enum {
				ProngIndex	 = TN,
				ReverseDepth = hfsm::detail::Max<Initial::ReverseDepth, Remaining::ReverseDepth>::Value,
				DeepWidth	 = hfsm::detail::Max<Initial::DeepWidth, Remaining::DeepWidth>::Value,
				StateCount	 = Initial::StateCount + Remaining::StateCount,
				ForkCount	 = Initial::ForkCount  + Remaining::ForkCount,
				ProngCount	 = Initial::ProngCount + Remaining::ProngCount,
			};

			Sub(ForkPointers& forkPointers);
			
			inline void wideLink(StateRegistry& stateRegistry,
								 const Index fork,
								 Parents& stateParents,
								 Parents& forkParents);

			inline void wideEnterInitial(Context& context, const Time time);

			inline void wideForwardRequest(const unsigned prong, const enum Transition::Type transition);
			inline void wideRequestRestart();
			inline void wideRequestResume(const unsigned stateIndex);

			inline void wideForwardSubstitute(const unsigned prong, Control& control, Context& context, const Time time) const;
			inline void wideSubstitute(const unsigned prong, Control& control, Context& context, const Time time) const;

			inline void wideChangeTo(const unsigned prong, Context& context, const Time time);
			inline void wideEnter(const unsigned prong, Context& context, const Time time);
			inline void wideLeave(const unsigned prong, Context& context, const Time time);

			inline bool wideUpdateAndTransition(const unsigned prong, Control& control, Context& context, const Time time);
			inline void wideUpdate(const unsigned prong, Context& context, const Time time);

			Initial initial;
		};

		//��������������������������������������������������������������

		template <unsigned TN, typename TI>
		struct Sub<TN, TI> {
			using Initial = TI;

			enum {
				ProngIndex	 = TN,
				ReverseDepth = Initial::ReverseDepth,
				DeepWidth	 = hfsm::detail::Max<1, Initial::DeepWidth>::Value,
				StateCount	 = Initial::StateCount,
				ForkCount	 = Initial::ForkCount,
				ProngCount	 = Initial::ProngCount,
			};

			Sub(ForkPointers& forkPointers);

			inline void wideLink(StateRegistry& stateRegistry,
								 const Index fork,
								 Parents& stateParents,
								 Parents& forkParents);

			inline void wideEnterInitial(Context& context, const Time time);

			inline void wideForwardRequest(const unsigned prong, const enum Transition::Type transition);
			inline void wideRequestRestart();
			inline void wideRequestResume(const unsigned stateIndex);

			inline void wideForwardSubstitute(const unsigned prong, Control& control, Context& context, const Time time) const;
			inline void wideSubstitute(const unsigned prong, Control& control, Context& context, const Time time) const;

			inline void wideChangeTo(const unsigned prong, Context& context, const Time time);
			inline void wideEnter(const unsigned prong, Context& context, const Time time);
			inline void wideLeave(const unsigned prong, Context& context, const Time time);

			inline bool wideUpdateAndTransition(const unsigned prong, Control& control, Context& context, const Time time);
			inline void wideUpdate(const unsigned prong, Context& context, const Time time);

			Initial initial;
		};

		using SubStates = Sub<0, TS...>;

#pragma endregion

		//��������������������������������������������������������������

    public:
		enum {
			ReverseDepth = SubStates::ReverseDepth + 1,
			DeepWidth	 = SubStates::DeepWidth,
			StateCount	 = State::StateCount + SubStates::StateCount,
			ForkCount	 = SubStates::ForkCount + 1,
			ProngCount	 = SubStates::ProngCount + sizeof...(TS),
			Width		 = sizeof...(TS),
		};

	//private:
		_C(ForkPointers& forkPointers);

		inline void deepLink(StateRegistry& stateRegistry,
							 const Parent parent,
							 Parents& stateParents,
							 Parents& forkParents);

		inline void deepEnterInitial(Context& context, const Time time);

		inline void deepForwardRequest(const enum Transition::Type transition);
		inline void deepRequestRestart();
		inline void deepRequestResume();

		inline void deepForwardSubstitute(Control& control, Context& context, const Time time) const;
		inline void deepSubstitute(Control& control, Context& context, const Time time) const;

		inline void deepChangeToRequested(Context& context, const Time time);
		inline void deepEnter(Context& context, const Time time);
		inline void deepLeave(Context& context, const Time time);

		inline bool deepUpdateAndTransition(Control& control, Context& context, const Time time);
		inline void deepUpdate(Context& context, const Time time);

	private:
		SubStates _subStates;

		HSFM_DEBUG_ONLY(const TypeInfo _type = TypeInfo::get<T>());
	};

#pragma endregion

	//----------------------------------------------------------------------

#pragma region Orthogonal

	template <typename T, typename... TS>
	class _O final
		: public _S<T>
	{
		template <typename, typename...>
		friend class _C;

		template <typename, typename...>
		friend class _O;

		template <typename>
		friend class _R;

		using State = _S<T>;

		//��������������������������������������������������������������

#pragma region Substate

		template <typename...>
		struct Sub;

		//��������������������������������������������������������������

		template <typename TI, typename... TR>
		struct Sub<TI, TR...>
			: Sub<TR...>
		{
			using Initial = TI;
			using Remaining = Sub<TR...>;

			enum {
				ReverseDepth = hfsm::detail::Max<Initial::ReverseDepth, Remaining::ReverseDepth>::Value,
				DeepWidth	 = Initial::DeepWidth + Remaining::DeepWidth,
				StateCount	 = Initial::StateCount + Remaining::StateCount,
				ForkCount	 = Initial::ForkCount  + Remaining::ForkCount,
				ProngCount	 = Initial::ProngCount + Remaining::ProngCount,
			};

			Sub(ForkPointers& forkPointers);

			inline void wideLink(StateRegistry& stateRegistry,
								 const Parent parent,
								 Parents& stateParents,
								 Parents& forkParents);

			inline void wideEnterInitial(Context& context, const Time time);

			inline void wideForwardRequest(const enum Transition::Type transition);
			inline void wideRequestRestart();
			inline void wideRequestResume();

			inline void wideForwardSubstitute(Control& control, Context& context, const Time time) const;
			inline void wideSubstitute(Control& control, Context& context, const Time time) const;

			inline void wideChangeToRequested(Context& context, const Time time);
			inline void wideEnter(Context& context, const Time time);
			inline void wideLeave(Context& context, const Time time);

			inline bool wideUpdateAndTransition(Control& control, Context& context, const Time time);
			inline void wideUpdate(Context& context, const Time time);

			Initial initial;
		};

		//��������������������������������������������������������������

		template <typename TI>
		struct Sub<TI> {
			using Initial = TI;

			enum {
				ReverseDepth = Initial::ReverseDepth,
				DeepWidth	 = Initial::DeepWidth,
				StateCount	 = Initial::StateCount,
				ForkCount	 = Initial::ForkCount,
				ProngCount	 = Initial::ProngCount,
			};

			Sub(ForkPointers& forkPointers);

			inline void wideLink(StateRegistry& stateRegistry,
								 const Parent parent,
								 Parents& stateParents,
								 Parents& forkParents);

			inline void wideEnterInitial(Context& context, const Time time);

			inline void wideForwardRequest(const enum Transition::Type transition);
			inline void wideRequestRestart();
			inline void wideRequestResume();

			inline void wideForwardSubstitute(Control& control, Context& context, const Time time) const;
			inline void wideSubstitute(Control& control, Context& context, const Time time) const;

			inline void wideChangeToRequested(Context& context, const Time time);
			inline void wideEnter(Context& context, const Time time);
			inline void wideLeave(Context& context, const Time time);

			inline bool wideUpdateAndTransition(Control& control, Context& context, const Time time);
			inline void wideUpdate(Context& context, const Time time);

			Initial initial;
		};

		using SubStates = Sub<TS...>;

#pragma endregion

		//����������������������������������������������������������������������
    public:
		enum {
			ReverseDepth = SubStates::ReverseDepth + 1,
			DeepWidth	 = SubStates::DeepWidth,
			StateCount	 = State::StateCount + SubStates::StateCount,
			ForkCount	 = SubStates::ForkCount,
			ProngCount	 = SubStates::ProngCount,
			Width		 = sizeof...(TS),
		};

	//private:
		_O(ForkPointers& forkPointers);

		inline void deepLink(StateRegistry& stateRegistry,
							 const Parent parent,
							 Parents& stateParents,
							 Parents& forkParents);

		inline void deepEnterInitial(Context& context, const Time time);

		inline void deepForwardRequest(const enum Transition::Type transition);
		inline void deepRequestRestart();
		inline void deepRequestResume();

		inline void deepForwardSubstitute(Control& control, Context& context, const Time time) const;
		inline void deepSubstitute(Control& control, Context& context, const Time time) const;

		inline void deepChangeToRequested(Context& context, const Time time);
		inline void deepEnter(Context& context, const Time time);
		inline void deepLeave(Context& context, const Time time);

		inline bool deepUpdateAndTransition(Control& control, Context& context, const Time time);
		inline void deepUpdate(Context& context, const Time time);

	private:
		SubStates _subStates;

		HSFM_DEBUG_ONLY(const TypeInfo _type = TypeInfo::get<T>());
	};

#pragma endregion

	//----------------------------------------------------------------------

#pragma region Root

	template <typename T>
	class _R final {
		using Apex = T;

		enum : unsigned {
			StateCapacity = (unsigned) 1.3 * Apex::StateCount,
			ForkCapacity  = Apex::ForkCount,
		};

		using StateRegistryImpl		 = StateRegistryT<StateCapacity>;
		using StateParentStorage	 = Array<Parent, Apex::StateCount>;
		using ForkParentStorage		 = Array<Parent, Apex::ForkCount>;
		using ForkPointerStorage	 = Array<Fork*, ForkCapacity>;
		using TransitionQueueStorage = Array<Transition, Apex::ForkCount>;

	public:
		enum {
			DeepWidth	= Apex::DeepWidth,
			StateCount	= Apex::StateCount,
			ForkCount	= Apex::ForkCount,
			ProngCount	= Apex::ProngCount,
			Width		= Apex::Width,
		};
		static_assert(StateCount < std::numeric_limits<Index>::max(), "Too many states in the hierarchy. Change 'Index' type.");

	public:
		_R(Context& context, const Time time = Time{});
		~_R();

		void update(const Time time);

		template <typename U>
		inline void changeTo()	{ _requests << Transition(Transition::Type::Restart,  TypeInfo::get<U>());	}

		template <typename U>
		inline void schedule()	{ _requests << Transition(Transition::Type::Schedule, TypeInfo::get<U>());	}

		template <typename U>
		inline void resume()	{ _requests << Transition(Transition::Type::Resume,   TypeInfo::get<U>());	}

		inline unsigned id(const Transition request) const	{ return _stateRegistry[*request.stateType]; }

	protected:
		void requestChange(const Transition request);
		void requestSchedule(const Transition request);

	private:
		Context& _context;

		StateRegistryImpl _stateRegistry;

		StateParentStorage _stateParents;
		ForkParentStorage  _forkParents;
		ForkPointerStorage _forkPointers;

		TransitionQueueStorage _requests;

		Apex _apex;
	};

	//----------------------------------------------------------------------

	class Control final {
		template <typename>
		friend class _R;

	public:
		Control(TransitionQueue& requests)
			: _requests(requests)
		{}

	public:
		template <typename T>
		inline void changeTo()	{ _requests << Transition(Transition::Type::Restart,  TypeInfo::get<T>());	}

		template <typename T>
		inline void schedule()	{ _requests << Transition(Transition::Type::Schedule, TypeInfo::get<T>());	}

		template <typename T>
		inline void resume()	{ _requests << Transition(Transition::Type::Resume,	  TypeInfo::get<T>());	}

		inline unsigned requestCount() const { return _requests.count();											}

	private:
		TransitionQueue& _requests;
	};

	//----------------------------------------------------------------------

#pragma endregion

public:

	using Base				= _B<Bare>;
	using TrackedBase		= _B<Tracked>;
	using TimedBase			= _B<Timed>;
	using TrackedTimedBase	= _B<Tracked, Timed>;

	template <typename... TInjections>
	using BaseT = _B<TInjections...>;

	//����������������������������������������������������������������������

	template <typename TClient>
	using State = _S<TClient>;

	//����������������������������������������������������������������������

	template <typename TClient, typename... TSubStates>
	using Composite	= _C<TClient, TSubStates...>;

	template <typename... TSubStates>
	using CompositePeers = _C<Base, TSubStates...>;

	//����������������������������������������������������������������������

	template <typename TClient, typename... TSubStates>
	using Orthogonal = _O<TClient, TSubStates...>;

	template <typename... TSubStates>
	using OrthogonalPeers = _O<Base, TSubStates...>;

	//����������������������������������������������������������������������

	template <typename TState>
	using Root = _R<TState>;

	template <typename... TSubStates>
	using CompositeRoot = _R<CompositePeers<TSubStates...>>;

	template <typename... TSubStates>
	using OrthogonalRoot = _R<OrthogonalPeers<TSubStates...>>;

	//----------------------------------------------------------------------
};

////////////////////////////////////////////////////////////////////////////////

}

#include "machine.inl"
