namespace hfsm {

////////////////////////////////////////////////////////////////////////////////

#pragma region Utility

template <typename TC, typename TD, unsigned TMS>
Machine<TC, TD, TMS>::Fork::Fork(const Index index,
								 const TypeInfo HSFM_DEBUG_ONLY(type_))
	: self(index)
#ifdef _DEBUG
	, type(type_)
#endif
{}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T>
unsigned
Machine<TC, TD, TMS>::StateRegistry::add() {
	return add(TypeInfo::get<T>());
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <unsigned TCapacity>
unsigned
Machine<TC, TD, TMS>::StateRegistryT<TCapacity>::add(const TypeInfo stateType) {
	const unsigned index = _typeToIndex.count();

	HSFM_CHECKED(_typeToIndex.insert(*stateType, index));

	return index;
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region State

template <typename TC, typename TD, unsigned TMS>
template <typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_B<TI, TR...>::widePreSubstitute(Context& context,
													   const Time time) const
{
	TI::preSubstitute(context, time);
	_B<TR...>::widePreSubstitute(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_B<TI, TR...>::widePreEnter(Context& context,
												  const Time time)
{
	TI::preEnter(context, time);
	_B<TR...>::widePreEnter(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_B<TI, TR...>::widePreUpdate(Context& context,
												   const Time time)
{
	TI::preUpdate(context, time);
	_B<TR...>::widePreUpdate(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_B<TI, TR...>::widePreTransition(Context& context,
													   const Time time) const
{
	TI::preTransition(context, time);
	_B<TR...>::widePreTransition(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_B<TI, TR...>::widePostLeave(Context& context,
												   const Time time)
{
	TI::postLeave(context, time);
	_B<TR...>::widePostLeave(context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename TI>
void
Machine<TC, TD, TMS>::_B<TI>::widePreSubstitute(Context& context,
												const Time time) const
{
	TI::preSubstitute(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename TI>
void
Machine<TC, TD, TMS>::_B<TI>::widePreEnter(Context& context,
										   const Time time)
{
	TI::preEnter(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename TI>
void
Machine<TC, TD, TMS>::_B<TI>::widePreUpdate(Context& context,
											const Time time)
{
	TI::preUpdate(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename TI>
void
Machine<TC, TD, TMS>::_B<TI>::widePreTransition(Context& context,
												const Time time) const
{
	TI::preTransition(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename TI>
void
Machine<TC, TD, TMS>::_B<TI>::widePostLeave(Context& context,
											const Time time)
{
	TI::postLeave(context, time);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

template <typename TC, typename TD, unsigned TMS>
template <typename T>
void
Machine<TC, TD, TMS>::_S<T>::deepLink(StateRegistry& stateRegistry,
									  const Parent parent,
									  Parents& stateParents,
									  Parents& /*forkParents*/)
{
	const auto id = stateRegistry.template add<Client>();
	stateParents[id] = parent;
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T>
void
Machine<TC, TD, TMS>::_S<T>::deepEnterInitial(Context& context,
											  const Time time)
{
	deepEnter(context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T>
bool
Machine<TC, TD, TMS>::_S<T>::deepSubstitute(Control& control,
											Context& context,
											const Time time) const
{
	const unsigned requestCountBefore = control.requestCount();

	_client.widePreSubstitute(context, time);
	_client.substitute(control, context, time);

	return requestCountBefore < control.requestCount();
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T>
void
Machine<TC, TD, TMS>::_S<T>::deepLeave(Context& context,
									   const Time time)
{
	_client.leave(context, time);
	_client.widePostLeave(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T>
void
Machine<TC, TD, TMS>::_S<T>::deepEnter(Context& context,
									   const Time time)
{
	_client.widePreEnter(context, time);
	_client.enter(context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T>
bool
Machine<TC, TD, TMS>::_S<T>::deepUpdateAndTransition(Control& control,
													 Context& context,
													 const Time time)
{
	_client.widePreUpdate(context, time);
	_client.update(context, time);

	const unsigned requestCountBefore = control.requestCount();

	_client.widePreTransition(context, time);
	_client.transition(control, context, time);

	return requestCountBefore < control.requestCount();
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T>
void
Machine<TC, TD, TMS>::_S<T>::deepUpdate(Context& context,
										const Time time)
{
	_client.widePreUpdate(context, time);
	_client.update(context, time);
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Composite::Substate

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI, typename... TR>
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI, TR...>::Sub(ForkPointers& forkPointers)
	: initial(forkPointers)
	, Remaining(forkPointers)
{}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI, TR...>::wideLink(StateRegistry& stateRegistry,
																 const Index fork,
																 Parents& stateParents,
																 Parents& forkParents)
{
	initial.deepLink(stateRegistry,
					 Parent(fork, ProngIndex, TypeInfo::get<T>(), TypeInfo::get<typename TI::Client>()),
					 stateParents,
					 forkParents);

	Remaining::wideLink(stateRegistry, fork, stateParents, forkParents);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI, TR...>::wideEnterInitial(Context& context,
																		 const Time time)
{
	initial.deepEnterInitial(context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI, TR...>::wideForwardRequest(const unsigned prong,
																		   const enum Transition::Type transition)
{
	if (ProngIndex == prong)
		initial.deepForwardRequest(transition);
	else
		Remaining::wideForwardRequest(prong, transition);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI, TR...>::wideRequestRestart() {
	initial.deepRequestRestart();
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI, TR...>::wideRequestResume(const unsigned prong) {
	if (ProngIndex == prong)
		initial.deepRequestResume();
	else
		Remaining::wideRequestResume(prong);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI, TR...>::wideForwardSubstitute(const unsigned prong,
																			  Control& control,
																			  Context& context,
																			  const Time time) const
{
	if (ProngIndex == prong)
		initial.deepForwardSubstitute(control, context, time);
	else
		Remaining::wideForwardSubstitute(prong, control, context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI, TR...>::wideSubstitute(const unsigned prong,
																	   Control& control,
																	   Context& context,
																	   const Time time) const
{
	if (ProngIndex == prong)
		initial.deepSubstitute(control, context, time);
	else
		Remaining::wideSubstitute(prong, control, context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI, TR...>::wideChangeTo(const unsigned prong,
																	 Context& context,
																	 const Time time)
{
	if (ProngIndex == prong)
		initial.deepChangeToRequested(context, time);
	else
		Remaining::wideChangeTo(prong, context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI, TR...>::wideEnter(const unsigned prong,
																  Context& context,
																  const Time time)
{
	if (ProngIndex == prong)
		initial.deepEnter(context, time);
	else
		Remaining::wideEnter(prong, context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI, TR...>::wideLeave(const unsigned prong,
																  Context& context,
																  const Time time)
{
	if (ProngIndex == prong)
		initial.deepLeave(context, time);
	else
		Remaining::wideLeave(prong, context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI, typename... TR>
bool
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI, TR...>::wideUpdateAndTransition(const unsigned prong,
																				Control& control,
																				Context& context,
																				const Time time)
{
	return ProngIndex == prong ?
		initial.deepUpdateAndTransition(control, context, time) :
		Remaining::wideUpdateAndTransition(prong, control, context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI, TR...>::wideUpdate(const unsigned prong,
																   Context& context,
																   const Time time)
{
	if (ProngIndex == prong)
		initial.deepUpdate(context, time);
	else
		Remaining::wideUpdate(prong, context, time);
}

#pragma endregion

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#pragma region Composite::Substate

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI>
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI>::Sub(ForkPointers& forkPointers)
	: initial(forkPointers)
{}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI>::wideLink(StateRegistry& stateRegistry,
														  const Index fork,
														  Parents& stateParents,
														  Parents& forkParents)
{
	initial.deepLink(stateRegistry,
					 Parent(fork, ProngIndex, TypeInfo::get<T>(), TypeInfo::get<typename TI::Client>()),
					 stateParents,
					 forkParents);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI>::wideEnterInitial(Context& context,
																  const Time time)
{
	initial.deepEnterInitial(context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI>::wideForwardRequest(const unsigned HSFM_ASSERT_ONLY(prong),
																	const enum Transition::Type transition)
{
	assert(ProngIndex == prong);

	initial.deepForwardRequest(transition);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI>::wideRequestRestart() {
	initial.deepRequestRestart();
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI>::wideRequestResume(const unsigned HSFM_ASSERT_ONLY(prong)) {
	assert(ProngIndex == prong);

	initial.deepRequestResume();
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI>::wideForwardSubstitute(const unsigned HSFM_ASSERT_ONLY(prong),
																	   Control& control,
																	   Context& context,
																	   const Time time) const
{
	assert(ProngIndex == prong);

	initial.deepForwardSubstitute(control, context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI>::wideSubstitute(const unsigned HSFM_ASSERT_ONLY(prong),
																Control& control,
																Context& context,
																const Time time) const
{
	assert(ProngIndex == prong);

	initial.deepSubstitute(control, context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI>::wideChangeTo(const unsigned HSFM_ASSERT_ONLY(prong),
															  Context& context,
															  const Time time)
{
	assert(ProngIndex == prong);

	initial.deepChangeToRequested(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI>::wideEnter(const unsigned HSFM_ASSERT_ONLY(prong),
														   Context& context,
														   const Time time)
{
	assert(ProngIndex == prong);

	initial.deepEnter(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI>::wideLeave(const unsigned HSFM_ASSERT_ONLY(prong),
														   Context& context,
														   const Time time)
{
	assert(ProngIndex == prong);

	initial.deepLeave(context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI>
bool
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI>::wideUpdateAndTransition(const unsigned HSFM_ASSERT_ONLY(prong),
																		 Control& control,
																		 Context& context,
																		 const Time time)
{
	assert(ProngIndex == prong);

	return initial.deepUpdateAndTransition(control, context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <unsigned TN, typename TI>
void
Machine<TC, TD, TMS>::_C<T, TS...>::Sub<TN, TI>::wideUpdate(const unsigned HSFM_ASSERT_ONLY(prong),
															Context& context,
															const Time time)
{
	assert(ProngIndex == prong);

	initial.deepUpdate(context, time);
}

#pragma endregion

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#pragma region Composite

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
Machine<TC, TD, TMS>::_C<T, TS...>::_C(ForkPointers& forkPointers)
	: Machine<TC, TD, TMS>::_C<T, TS...>::_S(forkPointers)
	, ForkT<T>(static_cast<Index>(forkPointers << this))
	, _subStates(forkPointers)
{}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_C<T, TS...>::deepLink(StateRegistry& stateRegistry,
											 const Parent parent,
											 Parents& stateParents,
											 Parents& forkParents)
{
	const auto id = stateRegistry.template add<Client>();
	stateParents[id] = parent;

	forkParents[this->self] = parent;

	_subStates.wideLink(stateRegistry, this->self, stateParents, forkParents);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_C<T, TS...>::deepEnterInitial(Context& context,
													 const Time time)
{
	State::deepEnter(context, time);

	this->active = 0;
	HSFM_DEBUG_ONLY(this->activeType = TypeInfo::get<typename SubStates::Initial::Client>());
	_subStates.wideEnterInitial(context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_C<T, TS...>::deepForwardRequest(const enum Transition::Type transition) {
	if (this->requested != INVALID_INDEX)
		_subStates.wideForwardRequest(this->requested, transition);
	else
		switch (transition) {
		case Transition::Restart:
			deepRequestRestart();
			break;

		case Transition::Resume:
			deepRequestResume();
			break;

		default:
			assert(false);
		}
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_C<T, TS...>::deepRequestRestart() {
	this->requested = 0;
	HSFM_DEBUG_ONLY(this->requestedType.clear());

	_subStates.wideRequestRestart();
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_C<T, TS...>::deepRequestResume() {
	this->requested = this->resumable != INVALID_INDEX ? this->resumable : 0;

#ifdef _DEBUG
	if (this->resumable != INVALID_INDEX)
		this->requestedType = this->resumableType;
	else
		this->requestedType.clear();
#endif

	_subStates.wideRequestResume(this->requested);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_C<T, TS...>::deepForwardSubstitute(Control& control,
														  Context& context,
														  const Time time) const
{
	assert(this->requested != INVALID_INDEX);

	if (this->requested == this->active)
		_subStates.wideForwardSubstitute(this->requested, control, context, time);
	else
		_subStates.wideSubstitute(this->requested, control, context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_C<T, TS...>::deepSubstitute(Control& control,
												   Context& context,
												   const Time time) const
{
	assert(this->requested != INVALID_INDEX);

	if (!State::deepSubstitute(control, context, time))
		_subStates.wideSubstitute(this->requested, control, context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_C<T, TS...>::deepChangeToRequested(Context& context,
														  const Time time)
{
	if (this->requested != INVALID_INDEX) {
		if (this->active == this->requested)
			_subStates.wideChangeTo(this->requested, context, time);
		else {
			_subStates.wideLeave(this->active, context, time);

			this->resumable = this->active;
			HSFM_DEBUG_ONLY(this->resumableType = this->activeType);

			this->active = this->requested;
			HSFM_DEBUG_ONLY(this->activeType = this->requestedType);

			this->requested = INVALID_INDEX;
			HSFM_DEBUG_ONLY(this->requestedType.clear());

			_subStates.wideEnter(this->active, context, time);
		}
	}
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_C<T, TS...>::deepEnter(Context& context,
											  const Time time)
{
	State::deepEnter(context, time);

	assert(this->active == INVALID_INDEX);
	std::swap(this->active, this->requested);
	_subStates.wideEnter(this->active, context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_C<T, TS...>::deepLeave(Context& context,
											  const Time time)
{
	_subStates.wideLeave(this->active, context, time);

	this->resumable = this->active;
	HSFM_DEBUG_ONLY(this->resumableType = this->activeType);

	this->active = INVALID_INDEX;
	HSFM_DEBUG_ONLY(this->activeType.clear());

	State::deepLeave(context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
bool
Machine<TC, TD, TMS>::_C<T, TS...>::deepUpdateAndTransition(Control& control,
															Context& context,
															const Time time)
{
	if (State::deepUpdateAndTransition(control, context, time)) {
		_subStates.wideUpdate(this->active, context, time);

		return true;
	} else
		return _subStates.wideUpdateAndTransition(this->active, control, context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_C<T, TS...>::deepUpdate(Context& context,
											   const Time time)
{
	State::deepUpdate(context, time);
	_subStates.wideUpdate(this->active, context, time);
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Orthogonal::Substate

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI, typename... TR>
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI, TR...>::Sub(ForkPointers& forkPointers)
	: initial(forkPointers)
	, Remaining(forkPointers)
{}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI, TR...>::wideLink(StateRegistry& stateRegistry,
															 const Parent parent,
															 Parents& stateParents,
															 Parents& forkParents)
{
	initial.deepLink(stateRegistry, parent, stateParents, forkParents);
	Remaining::wideLink(stateRegistry, parent, stateParents, forkParents);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI, TR...>::wideEnterInitial(Context& context,
																	 const Time time)
{
	initial.deepLink(this->parent, this->stateParents, this->forkParents);
	Remaining::wideLink(this->stateRegistry, this->parent, this->stateParents, this->forkParents);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI, TR...>::wideForwardRequest(const enum Transition::Type transition) {
	initial.deepForwardRequest(transition);
	Remaining::wideForwardRequest(transition);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI, TR...>::wideRequestRestart() {
	initial.deepRequestRestart();
	Remaining::wideRequestRestart();
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI, TR...>::wideRequestResume() {
	initial.deepRequestResume();
	Remaining::wideRequestResume();
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI, TR...>::wideForwardSubstitute(Control& control,
																		  Context& context,
																		  const Time time) const
{
	initial.deepForwardSubstitute(control, context, time);
	Remaining::wideForwardSubstitute(control, context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI, TR...>::wideSubstitute(Control& control,
																   Context& context,
																   const Time time) const
{
	initial.deepSubstitute(control, context, time);
	Remaining::wideSubstitute(control, context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI, TR...>::wideChangeToRequested(Context& context,
																		  const Time time)
{
	initial.deepChangeToRequested(context, time);
	Remaining::wideChangeToRequested(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI, TR...>::wideEnter(Context& context,
															  const Time time)
{
	initial.deepEnter(context, time);
	Remaining::wideEnter(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI, TR...>::wideLeave(Context& context,
															  const Time time)
{
	initial.deepLeave(context, time);
	Remaining::wideLeave(context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI, typename... TR>
bool
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI, TR...>::wideUpdateAndTransition(Control& control,
																			Context& context,
																			const Time time)
{
	return initial.deepUpdateAndTransition(control, context, time)
		|| Remaining::wideUpdateAndTransition(control, context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI, typename... TR>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI, TR...>::wideUpdate(Context& context,
															   const Time time)
{
	initial.deepUpdate(context, time);
	Remaining::wideUpdate(context, time);
}

#pragma endregion

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#pragma region Orthogonal::Substate

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI>
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI>::Sub(ForkPointers& forkPointers)
	: initial(forkPointers)
{}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI>::wideLink(StateRegistry& stateRegistry,
													  const Parent parent,
													  Parents& stateParents,
													  Parents& forkParents)
{
	initial.deepLink(stateRegistry, parent, stateParents, forkParents);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI>::wideEnterInitial(Context& context,
															  const Time time)
{
	initial.deepLink(this->parent, this->stateParents, this->forkParents);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI>::wideForwardRequest(const enum Transition::Type transition) {
	initial.deepForwardRequest(transition);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI>::wideRequestRestart() {
	initial.deepRequestRestart();
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI>::wideRequestResume() {
	initial.deepRequestResume();
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI>::wideForwardSubstitute(Control& control,
																   Context& context,
																   const Time time) const
{
	initial.deepForwardSubstitute(control, context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI>::wideSubstitute(Control& control,
															Context& context,
															const Time time) const
{
	initial.deepSubstitute(control, context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI>::wideChangeToRequested(Context& context,
																   const Time time)
{
	initial.deepChangeToRequested(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI>::wideEnter(Context& context,
													   const Time time)
{
	initial.deepEnter(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI>::wideLeave(Context& context,
													   const Time time)
{
	initial.deepLeave(context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI>
bool
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI>::wideUpdateAndTransition(Control& control,
																	 Context& context,
																	 const Time time)
{
	return initial.deepUpdateAndTransition(control, context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
template <typename TI>
void
Machine<TC, TD, TMS>::_O<T, TS...>::Sub<TI>::wideUpdate(Context& context,
														const Time time)
{
	initial.deepUpdate(context, time);
}

#pragma endregion

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#pragma region Orthogonal

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
Machine<TC, TD, TMS>::_O<T, TS...>::_O(ForkPointers& forkPointers)
	: Machine<TC, TD, TMS>::_O<T, TS...>::_S(forkPointers)
	, _subStates(forkPointers)
{}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_O<T, TS...>::deepLink(StateRegistry& stateRegistry,
											 const Parent parent,
											 Parents& stateParents,
											 Parents& forkParents)
{
	const auto id = stateRegistry.template add<typename Machine<TC, TD, TMS>::_O<T, TS...>::Client>();
	stateParents[id] = parent;

	_subStates.wideLink(stateRegistry, parent, stateParents, forkParents);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_O<T, TS...>::deepEnterInitial(Context& context,
													 const Time time)
{
	State::deepEnter(context, time);
	_subStates.wideEnterInitial(context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_O<T, TS...>::deepForwardRequest(const enum Transition::Type transition) {
	_subStates.wideForwardRequest(transition);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_O<T, TS...>::deepRequestRestart() {
	_subStates.wideRequestRestart();
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_O<T, TS...>::deepRequestResume() {
	_subStates.wideRequestResume();
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_O<T, TS...>::deepForwardSubstitute(Control& control,
														  Context& context,
														  const Time time) const
{
	_subStates.wideForwardSubstitute(control, context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_O<T, TS...>::deepSubstitute(Control& control,
												   Context& context,
												   const Time time) const
{
	if (!State::deepSubstitute(control, context, time))
		_subStates.wideSubstitute(control, context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_O<T, TS...>::deepChangeToRequested(Context& context,
														  const Time time)
{
	_subStates.wideChangeToRequested(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_O<T, TS...>::deepEnter(Context& context,
											  const Time time)
{
	State::deepEnter(context, time);
	_subStates.wideEnter(context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_O<T, TS...>::deepLeave(Context& context,
											  const Time time)
{
	_subStates.wideLeave(context, time);
	State::deepLeave(context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
bool
Machine<TC, TD, TMS>::_O<T, TS...>::deepUpdateAndTransition(Control& control,
															Context& context,
															const Time time)
{
	if (State::deepUpdateAndTransition(control, context, time)) {
		_subStates.wideUpdate(context, time);

		return true;
	} else
		return _subStates.wideUpdateAndTransition(control, context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T, typename... TS>
void
Machine<TC, TD, TMS>::_O<T, TS...>::deepUpdate(Context& context,
											   const Time time)
{
	State::deepUpdate(context, time);
	_subStates.wideUpdate(context, time);
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

#pragma region Root

template <typename TC, typename TD, unsigned TMS>
template <typename T>
Machine<TC, TD, TMS>::_R<T>::_R(Context& context,
								const Time time /*= Time{}*/)
	: _context(context)
	, _apex(_forkPointers)
{
	_apex.deepLink(_stateRegistry, Parent(), _stateParents, _forkParents);
	_apex.deepEnterInitial(_context, time);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T>
Machine<TC, TD, TMS>::_R<T>::~_R() {
	_apex.deepLeave(_context, Time{});
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T>
void
Machine<TC, TD, TMS>::_R<T>::update(const Time time) {
	Control control(_requests);
	_apex.deepUpdateAndTransition(control, _context, time);

	for (unsigned i = 0; i < MaxSubstitutions && _requests.count(); ++i) {
		unsigned changeCount = 0;

		for (const auto& transition : _requests)
			switch (transition.type) {
			case Transition::Restart:
			case Transition::Resume:
				requestChange(transition);
				++changeCount;
				break;

			case Transition::Schedule:
				requestSchedule(transition);
				break;

			default:
				assert(false);
			}
		_requests.clear();

		if (changeCount > 0) {
			Control substitutionControl(_requests);
			_apex.deepForwardSubstitute(substitutionControl, _context, time);
		}
	}

	_apex.deepChangeToRequested(_context, time);
}

//------------------------------------------------------------------------------

template <typename TC, typename TD, unsigned TMS>
template <typename T>
void
Machine<TC, TD, TMS>::_R<T>::requestChange(const Transition request) {
	const unsigned state = id(request);

	for (auto parent = _stateParents[state]; parent; parent = _forkParents[parent.fork]) {
		auto& fork = *_forkPointers[parent.fork];

		fork.requested = parent.prong;
		HSFM_DEBUG_ONLY(fork.requestedType = parent.prongType);
	}

	_apex.deepForwardRequest(request.type);
}

//������������������������������������������������������������������������������

template <typename TC, typename TD, unsigned TMS>
template <typename T>
void
Machine<TC, TD, TMS>::_R<T>::requestSchedule(const Transition request) {
	const unsigned state = id(request);

	const auto parent = _stateParents[state];
	auto& fork = *_forkPointers[parent.fork];

	fork.resumable = parent.prong;
	HSFM_DEBUG_ONLY(fork.requestedType = parent.prongType);

	// check that the fork isn't active
	HSFM_ASSERT_ONLY(const auto forksParent = _stateParents[fork.self]);
	HSFM_ASSERT_ONLY(const auto& forksFork = *_forkPointers[forksParent.fork]);
	assert(!forksFork.activeType);
}

#pragma endregion

////////////////////////////////////////////////////////////////////////////////

}
