#pragma once
#include <utils/boolean.h>
#include <atl/object.h>
#include <ts/generated.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/**                               Test Scenario                              **/
/******************************************************************************/

/** @typedef TS_Init
 * A test scenario initialization function.
 */
typedef bool (*TS_Init)(int, char**);

/** @typedef TS_Finish
 * A test scenario finalization function.
 */
typedef void (*TS_Finish)(void);

/** @typedef TS_GetState
 * A test scenario function that calculates a generalized state.
 */
typedef Object* (*TS_GetState)(void);

/** @typedef TS_GetStateId
 * A test scenario function that calculates a generalized state's identifier.
 */
typedef String* (*TS_GetStateId)(Object*);

/** @typedef TS_IsStationaryState
 * A test scenario function that checks whether the current state is stationary or not.
 */
typedef bool (*TS_IsStationaryState)(void);

/** @typedef TS_SaveModelState
 * A test scenario function that saves the current state into an object.
 */
typedef Object* (*TS_SaveModelState)(void);

/** @typedef TS_RestoreModelState
 * A test scenario function that restores the current state.
 */
typedef void (*TS_RestoreModelState)(Object*);

/** @typedef TS_ObserveState
 * A test scenario function that observes the current state.
 */
typedef void (*TS_ObserveState)(void);

/** @typedef TestScenario
 * A test scenario description.
 */
typedef struct TestScenario
{
    /// A test scenario name.
    const char*          name;
    /// A test scenario initialization function.
    TS_Init              init;
    /// A test scenario function that calculates a generalized state.
    TS_GetState          getState;
    /// A test scenario function that calculates a generalized state's identifier.
    TS_GetStateId        getStateId;
    /// A test scenario function that saves the current state into an object.
    TS_SaveModelState    saveModelState;
    /// A test scenario function that restores the current state.
    TS_RestoreModelState restoreModelState;
    /// A test scenario function that checks whether the current state is stationary or not.
    TS_IsStationaryState isStationaryState;
    /// A test scenario function that observes the current state.
    TS_ObserveState      observeState;
    /// A test scenario finalization function.
    TS_Finish            finish;
    /// A list of scenario functions.
    ScenarioFunctionID*  actions;
} TestScenario;

/** @fn String* getStateId_Default(Object *obj);
 * A default function that calculates a state's identifier.
 */
String* getStateId_Default(Object *obj);

#ifdef __cplusplus
} // extern "C" 
#endif
