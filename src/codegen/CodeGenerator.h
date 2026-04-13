#pragma once

#include "model/Model.h"

#include <map>
#include <ostream>
#include <string>
#include <vector>

/**
* @brief C code generator from models and a topological schedule.
*
* Accepts a model and schedule (a SID vector in topological order),
* a stream-by-stream C file with sections: includes a static structure,
* nwocg_generated_init(), nwocg_generated_step(), ext_ports[].
*/
class CodeGenerator
{
public:
	/**
	* @brief Constructor. Builds auxiliary indices m_sidToConnection and m_dstToConns.
	* @param model A fully parsed and validated model
	* @param schedule A topological schedule: a vector of SIDs in evaluation order
	*/
    explicit CodeGenerator(const Model & model, const std::vector<int> & schedule);

    /**
	* @brief Generates a complete C file to the out stream.
	* Calls emitIncludes → emitStruct → emitInitFunction → emitStepFunction → emitExtPorts.
	*/
    void emit(std::ostream & out);

private:
    void emitIncludes(std::ostream & out);     ///< #include section
    void emitStruct(std::ostream & out);       ///< static struct nwocg { ... }
    void emitInitFunction(std::ostream & out); ///< nwocg_generated_init() — resetting UnitDelay
    void emitStepFunction(std::ostream & out); ///< nwocg_generated_step() — calculations + UnitDelay flush
    void emitExtPorts(std::ostream & out);     ///< ext_ports[] + nwocg_generated_ext_ports

    /**
	* @brief Returns the C-signal identifier for the block with the given SID.
	* Priority: (1) Block name, (2) Line name, (3) "var_<SID>".
	* The result is cached in m_sidToSignal.
	*/
    std::string signalName(int sid);

    /**
	* @brief Formats a double as a C literal without extra zeros.
	* @param value Value. Must be finite; otherwise, exit(1).
	* @param sid SID of the block for error diagnostics.
	*/
    std::string formatDouble(double value, int sid);

    /**
	* @brief Builds a C expression to calculate the block output.
	* Supports SUM and GAIN. INPORT/OUTPORT/UNIT_DELAY return "0".
	*/
    std::string buildExpression(const Block & block);

    const Model & m_model;                          ///< Link to model (not owned)
    const std::vector<int> & m_schedule;            ///< Link to schedule (not owned)
    std::map<int, std::string> m_sidToSignal;       ///< Cache: SID → C-identifier
    std::map<int, const Connection *> m_sidToConnection; ///< Source SID → First Outgoing Connection
    std::map<int, std::vector<const Connection *>> m_dstToConns; ///< Recipient SID → Incoming Connection, sorted by dstPort
};
