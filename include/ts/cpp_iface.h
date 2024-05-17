#ifndef _CPP_IFACE_H
#define _CPP_IFACE_H

#include <ts/engine_tsi.h>
#include <ts/engine.h>
#include <ts/scenario.h>

// Служебные функции для вызова С++ функторов
typedef void (*InitArcSymbol)( ArcSymbolExecutor* executor, ArcSymbol* symbol );
typedef void (*NextArcSymbol)( ArcSymbolExecutor* executor, ArcSymbol* next_symbol, int kind, Object* value );
typedef bool (*ExecuteArcSymbol)( ArcSymbolExecutor* executor, int arcid, ArcSymbol* symbol, ArcSymbol* next_symbol );

struct ArcSymbol_Iface
{
	InitArcSymbol init;
	NextArcSymbol next;
	ExecuteArcSymbol execute;
};

// Служебные методы для вызова С++ функторов
typedef Object* (*Model_state_save)(ModelStateManager* manager);
typedef void (*Model_state_restore)(ModelStateManager* manager, Object* state);
typedef bool (*Model_state_is_stationary)(ModelStateManager* manager);
typedef void (*Model_state_observe)(ModelStateManager* manager);

struct ModelStateManager_Iface
{
	Model_state_save save;
	Model_state_restore restore;
	Model_state_is_stationary is_stationary;
	Model_state_observe observe;
};
/********************************************************************/
/* Utility functions to operate with C++ scenarios                  */
/*******************************************************************/
typedef int      (*Scenario_init)(TestScenario* ts, int argc, char** argv);
typedef void     (*Scenario_finish)(TestScenario* ts);
typedef Object * (*Scenario_get_state)(TestScenario* ts);
typedef String * (*Scenario_get_state_id)(TestScenario* ts, Object* state);
typedef Object * (*Scenario_save_model_state)(TestScenario* ts);
typedef void     (*Scenario_restore_model_state)(TestScenario* ts, Object * obj);
typedef int      (*Scenario_is_stationary_state)(TestScenario* ts);
typedef void     (*Scenario_observe_state)(TestScenario* ts);

typedef void (*Scenario_set_default_get_state)(TestScenario* ts);
typedef void (*Scenario_set_default_get_state_id)(TestScenario* ts);

struct Scenario_Iface
{
	Scenario_init init;
	Scenario_finish finish;
	Scenario_get_state get_state;
	Scenario_get_state_id get_state_id;
	Scenario_save_model_state save_model_state;
	Scenario_restore_model_state restore_model_state;
	Scenario_is_stationary_state is_stationary;
	Scenario_observe_state observe;

	Scenario_set_default_get_state set_default_get_state;
	Scenario_set_default_get_state_id set_default_get_state_id;
};

typedef const char * (*Action_name)(ScenarioFunctionID action);
typedef Object *     (*Action_init)(ScenarioFunctionID action);
typedef int          (*Action_has_next)(ScenarioFunctionID action, Object * par);
typedef int          (*Action_next)(ScenarioFunctionID action, Object * par);
typedef int          (*Action_invoke)(ScenarioFunctionID action, Object * par);

struct Action_Iface
{
	Action_name name;
	Action_init init;
	Action_has_next has_next;
	Action_next next;
	Action_invoke invoke;
};

// 
// Общий интерфейс расширения для С++
//

struct Cpp_Iface
{
	struct Scenario_Iface IScenario;
	struct Action_Iface IAction;
	struct ModelStateManager_Iface IModelStateManager;
	struct ArcSymbol_Iface IArcSymbol;
};

#ifdef __cplusplus
extern "C" 
{
#endif

bool is_cplusplus_mode(void) ;
void set_cplusplus_iface(struct Cpp_Iface* iface) ;
struct Cpp_Iface* get_cplusplus_iface(void) ;

// Обертки для вызова С++ интерфейса
// void initArcSymbol( ArcSymbolExecutor* executor, ArcSymbol* symbol );
// void nextArcSymbol( ArcSymbolExecutor* executor, ArcSymbol* next_symbol, int kind, Object* value );
// bool executeArcSymbol( ArcSymbolExecutor* executor, int arcid, ArcSymbol* symbol, ArcSymbol* next_symbol );

Object* model_state_save(ModelStateManager* manager);
void model_state_restore(ModelStateManager* manager, Object* state);
bool model_state_is_stationary(ModelStateManager* manager);
void model_state_observe(ModelStateManager* manager);

int      scenario_init(TestScenario* ts, int argc, char** argv);
void     scenario_finish(TestScenario* ts);
Object * scenario_get_state(TestScenario* ts);
String * scenario_get_state_id(TestScenario* ts, Object* state);
Object * scenario_save_model_state(TestScenario* ts);
void     scenario_restore_model_state(TestScenario* ts, Object * obj);
int      scenario_is_stationary_state(TestScenario* ts);
void     scenario_observe_state(TestScenario* ts);

void scenario_set_default_get_state(TestScenario* ts);
String* getStateId_Default(Object* obj);
void scenario_set_default_get_state_id(TestScenario* ts);

const char * action_name(ScenarioFunctionID action);
Object *     action_init(ScenarioFunctionID action);
int          action_has_next(ScenarioFunctionID action, Object * par);
int          action_next(ScenarioFunctionID action, Object * par);
int          action_invoke(ScenarioFunctionID action, Object * par);

#ifdef __cplusplus
}
#endif

#endif // _CPP_IFACE_H