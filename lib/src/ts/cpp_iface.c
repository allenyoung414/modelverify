#include <ts/cpp_iface.h>

void initArcSymbol( ArcSymbolExecutor* executor, ArcSymbol* symbol );
void nextArcSymbol( ArcSymbolExecutor* executor, ArcSymbol* next_symbol, int kind, Object* value );
bool executeArcSymbol( ArcSymbolExecutor* executor, int arcid, ArcSymbol* symbol, ArcSymbol* next_symbol );

Object* model_state_save(ModelStateManager* manager)
{
	if (is_cplusplus_mode())
	{
		return get_cplusplus_iface()->IModelStateManager.save(manager);
	}
	else 
	{
		return manager->saveModelState();
	}
}
void model_state_restore(ModelStateManager* manager, Object* state)
{
	if (is_cplusplus_mode())
	{
		 get_cplusplus_iface()->IModelStateManager.restore(manager, state);
	}
	else 
	{
		 manager->restoreModelState(state);
	}
}
bool model_state_is_stationary(ModelStateManager* manager)
{
	if (is_cplusplus_mode())
	{
		return get_cplusplus_iface()->IModelStateManager.is_stationary(manager);
	}
	else 
	{
		return manager->isStationaryState();
	}
}
void model_state_observe(ModelStateManager* manager)
{
	if (is_cplusplus_mode())
	{
		 get_cplusplus_iface()->IModelStateManager.observe(manager);
	}
	else 
	{
		 manager->observeState();
	}
}

int      scenario_init(TestScenario* ts, int argc, char** argv)
{
	if (is_cplusplus_mode())
	{
		return get_cplusplus_iface()->IScenario.init(ts, argc, argv);
	}
	else 
	{
		return ts->init(argc, argv);
	}
}
void     scenario_finish(TestScenario* ts)
{
	if (is_cplusplus_mode())
	{
		 get_cplusplus_iface()->IScenario.finish(ts);
	}
	else 
	{
		 ts->finish();
	}
}
Object * scenario_get_state(TestScenario* ts)
{
	if (is_cplusplus_mode())
	{
		return get_cplusplus_iface()->IScenario.get_state(ts);
	}
	else 
	{
		return ts->getState();
	}
}
String * scenario_get_state_id(TestScenario* ts, Object* state)
{
	if (is_cplusplus_mode())
	{
		return get_cplusplus_iface()->IScenario.get_state_id(ts, state);
	}
	else 
	{
		return ts->getStateId(state);
	}
}
Object * scenario_save_model_state(TestScenario* ts)
{
	if (is_cplusplus_mode())
	{
		return get_cplusplus_iface()->IScenario.save_model_state(ts);
	}
	else 
	{
		return ts->saveModelState();
	}
}
void     scenario_restore_model_state(TestScenario* ts, Object * obj)
{
	if (is_cplusplus_mode())
	{
		 get_cplusplus_iface()->IScenario.restore_model_state(ts, obj);
	}
	else 
	{
		 ts->restoreModelState(obj);
	}
}
int      scenario_is_stationary_state(TestScenario* ts)
{
	if (is_cplusplus_mode())
	{
		return get_cplusplus_iface()->IScenario.is_stationary(ts);
	}
	else 
	{
		return ts->isStationaryState();
	}
}
void     scenario_observe_state(TestScenario* ts)
{
	if (is_cplusplus_mode())
	{
		 get_cplusplus_iface()->IScenario.observe(ts);
	}
	else 
	{
		 ts->observeState();
	}
}

String* getStateId_Default(Object *obj)
{
    return toString(obj);
}

void scenario_set_default_get_state(TestScenario* ts)
{
	if (is_cplusplus_mode())
	{
		 get_cplusplus_iface()->IScenario.set_default_get_state(ts);
	}
	else 
	{
		ts->getState = singletonState;
	}
}
void scenario_set_default_get_state_id(TestScenario* ts)
{
	if (is_cplusplus_mode())
	{
		 get_cplusplus_iface()->IScenario.set_default_get_state_id(ts);
	}
	else 
	{
		ts->getStateId = getStateId_Default;
	}
}

const char * action_name(ScenarioFunctionID action)
{
	if (is_cplusplus_mode())
	{
		return get_cplusplus_iface()->IAction.name(action);
	}
	else 
	{
		return action->name;
	}
}
Object *     action_init(ScenarioFunctionID action)
{
	if (is_cplusplus_mode())
	{
		return get_cplusplus_iface()->IAction.init(action);
	}
	else 
	{
		return action->init();
	}
}
int          action_has_next(ScenarioFunctionID action, Object * par)
{
	if (is_cplusplus_mode())
	{
		return get_cplusplus_iface()->IAction.has_next(action, par);
	}
	else 
	{
		return action->has_next(par);
	}
}
int          action_next(ScenarioFunctionID action, Object * par)
{
	if (is_cplusplus_mode())
	{
		return get_cplusplus_iface()->IAction.next(action, par);
	}
	else 
	{
		return action->next(par);
	}
}
int          action_invoke(ScenarioFunctionID action, Object * par)
{
	if (is_cplusplus_mode())
	{
		return get_cplusplus_iface()->IAction.invoke(action, par);
	}
	else 
	{
		return action->call(par);
	}
}

static struct Cpp_Iface * _cpp_iface = 0;

bool is_cplusplus_mode(void) { return _cpp_iface != 0; }
void set_cplusplus_iface(struct Cpp_Iface* iface) { _cpp_iface = iface; }
struct Cpp_Iface* get_cplusplus_iface(void) { return _cpp_iface; }
