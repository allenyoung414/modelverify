/*
 * Copyright 2002-2009 ISP RAS (http://www.ispras.ru), UniTESK Lab (http://www.unitesk.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * File: TraceXMLFormatter.c
 * Description:
 *       This file contains an implementation of TraceXMLFormatter.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TraceXMLFormatter.h"
#include "TraceBool.h"
#include "TraceList.h"
#include <atl/stringbuffer.h>
#include <utils/assertion.h>


/*
 * Encode attribute values
 */
static String * encodeAttValue(const char *value)
{
unsigned i, last, len;
StringBuffer * buff;
char buff2[20];

  len = strlen(value);
  buff = createCustomized_StringBuffer( 2 * len + 1 );
  if(buff == NULL) return NULL;

  last = 0;
  for(i=0; i<len; i++) {
	  if ((/*signed*/ unsigned char)value[i] < 32) {
        appendCharArray_StringBuffer( r(buff), value+last, i-last);
	    sprintf(buff2, "&amp;#x%02X;", value[i]);
        append_StringBuffer( r(buff), buff2);
        last = i+1;
      } else if (value[i] == '<') {
        appendCharArray_StringBuffer( r(buff), value+last, i-last);
        append_StringBuffer( r(buff), "&lt;" );
        last = i+1;
      } else if (value[i] == '&') {
        appendCharArray_StringBuffer( r(buff), value+last, i-last);
        append_StringBuffer( r(buff), "&amp;" );
        last = i+1;
        break;
      } else if (value[i] == '"') {
        appendCharArray_StringBuffer( r(buff), value+last, i-last);
        append_StringBuffer( r(buff), "&quot;" );
        last = i+1;
	  }
   }
  appendCharArray_StringBuffer( r(buff), value+last, len-last );

  return toString(buff);
}

/*
 * Encode CDATA values. Do not affect '<' and '>', just escape '&', dangerous ']'
 * and non-printable characters.
 */
static String * encodeCdataValue(const char *value)
{
unsigned i, last, len;
StringBuffer * buff;
char buff2[10];

//char extraEscaping = 0;
//  if ( memcmp(value, "<value><string_value>", 21) != 0 )	extraEscaping = 1;

  len = strlen(value);
  buff = createCustomized_StringBuffer( 2 * len + 1 );
  if(buff == NULL) return NULL;

  last = 0;
  for(i=0; i<len; i++) {
//	if (value[i] == '>') {
//      StringBuffer_append(buff, value+last, i-last);
//      StringBuffer_append_s(buff, "&#x3E;");
//      last = i+1;
//	} else

	if (value[i] == ']' && value[i+1] == ']' && value[i+2] == '>') {
      appendCharArray_StringBuffer( r(buff), value+last, i-last);
      append_StringBuffer( r(buff), "]]&gt;");
      last = i+3;
	} else if (value[i] == '&') {
      appendCharArray_StringBuffer( r(buff), value+last, i-last);
//      append_StringBuffer( r(buff), extraEscaping ? "&#x26;amp;" : "&amp;" );
      append_StringBuffer( r(buff), "&amp;" );
      last = i+1;
	} else if ((/*signed*/ unsigned char)value[i] < 32
		&& value[i] != '\n' && value[i] != '\t' && value[i] != '\r')
	{
      appendCharArray_StringBuffer( r(buff), value+last, i-last);
	  sprintf(buff2, "&#x%02X;", value[i]);
      append_StringBuffer( r(buff), buff2);
      last = i+1;
	}
  }
  appendCharArray_StringBuffer( r(buff), value+last, i-last);

  return toString(buff);
}

/*
 * Encode boolean value
 */
static char * encodeBoolValue(TraceBool value)
{
  return (value == tr_true) ? "true" : "false";
}

/*
 * Indent methods
 */

/** Append indent to string buffer */
void Indent_print(Indent ind, StringBuffer *ss)
{
char *buf;
char buf2[128];

  assertion(ind.cur_indent >= 0, "Invalid indent");
  if (ind.cur_indent == 0)	return;

  if (ind.cur_indent < sizeof(buf2))	buf = buf2;	// Usual situation
  else	buf = (char *)malloc(ind.cur_indent + 1);

  if(buf == NULL)	return;

  memset(buf, ind.indent_char, ind.cur_indent);
  buf[ind.cur_indent] = 0;
  append_StringBuffer( r(ss), buf );

  if (buf != buf2)	free(buf);
}


/*
 * TraceXMLFormatter methods definition
 */
TraceXMLFormatter_state * TraceXMLFormatter_state_create(int indent_size, char indent_char)
{
TraceXMLFormatter_state * tfs;

  tfs = (TraceXMLFormatter_state *)malloc(sizeof(*tfs));
  if(tfs != NULL) {
    tfs->indent_size = indent_size;
    tfs->indent_char = indent_char;
    tfs->cur_indent = 0;
    tfs->indent.indent_size = indent_size;
    tfs->indent.indent_char = indent_char;
    tfs->indent.cur_indent = 0;
  }
  return tfs;
}
///////////////////////////////////
void TraceXMLFormatter_state_delete(void * tfs) {
  if(tfs != NULL) free(tfs);
}
///////////////////////////////////
TraceFormatter * TraceXMLFormatter_default_create() {
    return TraceXMLFormatter_create(2, ' ');
}
///////////////////////////////////
TraceBool TraceXMLFormatter_valid(TraceFormatter *tf)
{
  if( (NULL == tf->state) ||
      (NULL == tf->delete_state) ||
      (NULL == tf->startTrace) ||
      (NULL == tf->endTrace) ||
      (NULL == tf->traceScenarioStart) ||
      (NULL == tf->traceEnvironmentProperties) ||
      (NULL == tf->traceScenarioParameters) ||
      (NULL == tf->traceTestEngineProperty) ||
      (NULL == tf->traceScenarioEnd) ||
      (NULL == tf->traceScenarioValue) ||
      (NULL == tf->traceState) ||
      (NULL == tf->traceTransitionStart) ||
      (NULL == tf->traceTransitionEnd) ||
      (NULL == tf->traceModelOperationStart) ||
      (NULL == tf->traceModelOperationArgument) ||
      (NULL == tf->traceModelOperationResult) ||
      (NULL == tf->traceModelOperationChannel) ||
      (NULL == tf->traceModelOperationTimestamp) ||
      (NULL == tf->traceOracleStart) ||
      (NULL == tf->tracePreconditionEnd) ||
      (NULL == tf->traceOracleEnd) ||
      (NULL == tf->traceModelOperationEnd) ||
      (NULL == tf->traceSeriesStart) ||
      (NULL == tf->traceSeriesEnd) ||
      (NULL == tf->tracePrimeFormula) ||
      (NULL == tf->traceCoverageElement) ||
      (NULL == tf->traceMark) ||
      (NULL == tf->traceCoverageStructureStart) ||
      (NULL == tf->traceFormulaeStart) ||
      (NULL == tf->traceFormula) ||
      (NULL == tf->traceFormulaeEnd) ||
      (NULL == tf->traceCoverageStart) ||
      (NULL == tf->traceElement) ||
      (NULL == tf->traceCoverageEnd) ||
      (NULL == tf->traceCoverageStructureEnd) ||
      (NULL == tf->traceException) ||
      (NULL == tf->traceInterimException) ||
      (NULL == tf->traceInternalError) ||
      (NULL == tf->traceSystemInfo) ||
      (NULL == tf->traceUserInfo) )
    return tr_false;
  return tr_true;
}



/*
 * Trace level methods
 */

String * TraceXMLFormatter_startTrace(void *state, const char *encoding)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer *ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

  // Reinitialize indent
  st->indent.indent_size = st->indent_size;
  st->indent.indent_char = st->indent_char;
  st->indent.cur_indent = st->cur_indent;

  if (strcmp(encoding,"")) {
	append_StringBuffer( r(ss), "<?xml version=\"1.0\" encoding=\"" );
    append_StringBuffer( r(ss), encoding );
    append_StringBuffer( r(ss), "\"?>\n" );
  }

  Indent_print(st->indent, ss);
  st->indent.cur_indent += st->indent.indent_size;
  append_StringBuffer( r(ss), "<trace version=\"");
  append_StringBuffer( r(ss), TRACE_VERSION);
  append_StringBuffer( r(ss), "\">\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_endTrace(void *state, TraceBool original)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

  st->indent.cur_indent -= st->indent.indent_size;
  Indent_print(st->indent, ss);
  append_StringBuffer( r(ss), "</trace>\n");
  if (!original)
	  st->indent.cur_indent += st->indent.indent_size;

  return toString(ss);
}


/*
 * Scenario level methods
 */
String * TraceXMLFormatter_traceScenarioValue(void *state, int traceId, const char *kind, const char *type, const char *name, const char *value);

String * TraceXMLFormatter_traceScenarioStart(void *state, int traceId, const char *name, time_t now, const char *host, const char *os)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent++ << "<scenario_start" << " trace=\"" << traceId << "\""
//     << " name=\"" << encodeAttValue(name) << "\"" << " time=\"" << now << "000\""
//     << " host=\"" << encodeAttValue(host) << "\"" << " os=\"" << encodeAttValue(os)
//     << "\"" << "/>" << endl;

  Indent_print(st->indent, ss);
  st->indent.cur_indent += st->indent.indent_size;
  append_StringBuffer(       r(ss), "<scenario_start trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId );
  append_StringBuffer(       r(ss), "\" name=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(name));
  append_StringBuffer(       r(ss), "\" time=\"");
  appendFormat_StringBuffer( r(ss), "%ld", (long)now );
  append_StringBuffer(       r(ss), "000\" host=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(host));
  append_StringBuffer(       r(ss), "\" os=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(os));
  append_StringBuffer(       r(ss), "\"/>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceEnvironmentProperties(void *state, int traceId, TraceList *properties) //  map<char *,char *>
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer *ss;
  TracePair *prop;
  TraceListitem *item;

  if(properties == NULL) return NULL;

  ss = create_StringBuffer();
  if(st == NULL || ss == NULL)	return NULL;

  for(item = properties->head;(item != NULL); item = item->next)
  {
//    ss << indent << "<environment" << " trace=\"" << traceId << "\""
//       << " name=\"" << encodeAttValue(it->first) << "\"" << " value=\""
//       << encodeAttValue(it->second) << "\"" << "/>" << endl;
    prop = (TracePair *)item->data;
    if(!prop) continue;
    Indent_print(st->indent, ss);
    append_StringBuffer(       r(ss), "<environment trace=\"");
    appendFormat_StringBuffer( r(ss), "%d", traceId);
    append_StringBuffer(       r(ss), "\" name=\"");
    appendString_StringBuffer( r(ss), encodeAttValue(prop->first) );
    append_StringBuffer(       r(ss), "\" value=\"");
    appendString_StringBuffer( r(ss), encodeAttValue(prop->second) );
    append_StringBuffer(       r(ss), "\"/>\n");
  }

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceScenarioParameters(void *state, int traceId, int argc, const char** argv)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  int i;
  char name[10];
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

  for(i = 0; i<argc; i++) {
    sprintf(name, "arg%d", i);
    appendString_StringBuffer( r(ss), TraceXMLFormatter_traceScenarioValue(state, traceId, "parameter", "String", name, argv[i]) );
   }

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceTestEngineProperty(void *state, int traceId, const char *name, const char *value)
{
  return TraceXMLFormatter_traceScenarioValue(state, traceId, "test engine property", "String", name, value );
}
///////////////////////////////////
String * TraceXMLFormatter_traceScenarioEnd(void *state, int traceId, time_t now)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << --indent << "<scenario_end" << " trace=\"" << traceId << "\"" << " time=\"" << now << "000\"" << "/>" << endl;

  st->indent.cur_indent -= st->indent.indent_size;
  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<scenario_end trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId );
  append_StringBuffer(       r(ss), "\" time=\"");
  appendFormat_StringBuffer( r(ss), "%ld", (long)now );
  append_StringBuffer(       r(ss), "000\"/>\n");

  return toString(ss);
}


/*
 * Transition level methods
 */

String * TraceXMLFormatter_traceScenarioValue(void *state, int traceId, const char *kind, const char *type, const char *name, const char *value)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();
  if(st == NULL || ss == NULL)	return NULL;


//  ss << indent << "<scenario_value" << " trace=\"" << traceId << "\"" << " kind=\"" << encodeAttValue(kind)
//     << "\"" << " type=\"" << encodeAttValue(type) << "\"" << " name=\"" << encodeAttValue(name) << "\""
//     << ">" << "<![CDATA[" << value << "]]>" << "</scenario_value>" << endl;

  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<scenario_value trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\" kind=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(kind) );
  append_StringBuffer(       r(ss), "\" type=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(type) );
  append_StringBuffer(       r(ss), "\" name=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(name) );
  append_StringBuffer(       r(ss), "\"><![CDATA[");
  appendString_StringBuffer( r(ss), encodeCdataValue(value) );
  append_StringBuffer(       r(ss), "]]></scenario_value>\n");

  return toString(ss);
}

///////////////////////////////////
// TODO: type = ?
String * TraceXMLFormatter_traceState(void *state, int traceId, const char *id)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent << "<state" << " trace=\"" << traceId << "\""
//     << " id=\"" << encodeAttValue(id) << "\"" << "/>" << endl;

  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<state trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\" id=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(id) );
  append_StringBuffer(       r(ss), "\"/>\n");

  return toString(ss);
}
///////////////////////////////////
// TODO: type = ?
// TODO: "scenario method" or "scenario function"?
// TODO: "iteration variable" or something else?
// TODO: name and type of "iteration variable" are unknown
String * TraceXMLFormatter_traceTransitionStart (void *state, int traceId, const char *id)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent++ << "<transition_start" << " trace=\"" << traceId << "\""
//     << " id=\"" << encodeAttValue(id) << "\"" << "/>" << endl;

  Indent_print(st->indent, ss);
  st->indent.cur_indent += st->indent.indent_size;
  append_StringBuffer(       r(ss), "<transition_start trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\" id=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(id) );
  append_StringBuffer(       r(ss), "\"/>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceTransitionEnd (void *state, int traceId)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << --indent << "<transition_end" << " trace=\"" << traceId << "\"" << "/>" << endl;

  st->indent.cur_indent -= st->indent.indent_size;
  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<transition_end trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\"/>\n");

  return toString(ss);
}


/*
 * Oracle level methods
 */
String * TraceXMLFormatter_traceModelOperationStart(void *state, int traceId, const char *kind, const char *subsystem, const char *signature, unsigned refid)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent++ << "<model_operation_start" << " trace=\"" << traceId << "\"" << " kind=\"" << encodeAttValue(kind) << "\"";
//  ss << " package=\"" << encodeAttValue(subsystem) << "\"";
//  ss << " signature=\"" << encodeAttValue(signature) << "\"" << " refid=\"" << refid << "\"" << "/>" << endl;

  Indent_print(st->indent, ss);
  st->indent.cur_indent += st->indent.indent_size;
  append_StringBuffer(       r(ss), "<model_operation_start trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\" kind=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(kind) );
  append_StringBuffer(       r(ss), "\"");

  if (strlen(subsystem) > 0) {
    append_StringBuffer(       r(ss), " package=\"");
    appendString_StringBuffer( r(ss), encodeAttValue(subsystem) );
    append_StringBuffer(       r(ss), "\"");
  }

  append_StringBuffer(       r(ss), " signature=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(signature) );
  append_StringBuffer(       r(ss), "\" refid=\"");
  appendFormat_StringBuffer( r(ss), "%u", refid);
  append_StringBuffer(       r(ss), "\"/>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceModelOperationArgument(void *state, int traceId, const char *type, const char *name, const char *value)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent << "<model_value" << " trace=\"" << traceId << "\"" << " kind=\"argument\""
//     << " type=\"" << encodeAttValue(type) << "\"" << " name=\"" << encodeAttValue(name) << "\""
//     << ">" << "<![CDATA[" << value << "]]>" << "</model_value>" << endl;

  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<model_value trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\" kind=\"argument\" type=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(type) );
  append_StringBuffer(       r(ss), "\" name=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(name) );
  append_StringBuffer(       r(ss), "\"><![CDATA[");
  appendString_StringBuffer( r(ss), encodeCdataValue(value) );
  append_StringBuffer(       r(ss), "]]></model_value>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceModelOperationResult(void *state, int traceId, const char *type, const char *value)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer *ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent << "<model_value" << " trace=\"" << traceId << "\"" << " kind=\"result\""
//     << " type=\"" << encodeAttValue(type) << "\"" << "><![CDATA[" << value << "]]>" << "</model_value>" << endl;

  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<model_value trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\" kind=\"result\" type=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(type) );
  append_StringBuffer(       r(ss), "\"><![CDATA[");
  appendString_StringBuffer( r(ss), encodeCdataValue(value) );
  append_StringBuffer(       r(ss), "]]></model_value>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceModelOperationChannel(void *state, int traceId, const char *channel)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent << "<model_value" << " trace=\"" << traceId << "\"" << " kind=\"channel\""
//     << " type=\"ChannelID\"" << "><![CDATA[" << channel << "]]>" << "</model_value>" << endl;

  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<model_value trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\" kind=\"channel\" type=\"ChannelID\"><![CDATA[");
  appendString_StringBuffer( r(ss), encodeCdataValue(channel) );
  append_StringBuffer(       r(ss), "]]></model_value>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceModelOperationTimestamp(void *state, int traceId, const char *timestamp)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent << "<model_value" << " trace=\"" << traceId << "\"" << " kind=\"timestamp\""
//     << " type=\"TimeInterval\"" << "><![CDATA[" << timestamp << "]]>" << "</model_value>" << endl;

  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<model_value trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\" kind=\"timestamp\" type=\"TimeInterval\"><![CDATA[");
  appendString_StringBuffer( r(ss), encodeCdataValue(timestamp) );
  append_StringBuffer(       r(ss), "]]></model_value>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceOracleStart(void *state, int traceId, const char *subsystem, const char *signature, long ref)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent++ << "<oracle_start" << " trace=\"" << traceId << "\""
//     << " package=\"" << encodeAttValue(subsystem) << "\"";
//     << " signature=\"" << encodeAttValue(signature) << "\"";
//     << " ref=\"" << ref << "\"" << "/>"<< endl;

  Indent_print(st->indent, ss);
  st->indent.cur_indent += st->indent.indent_size;
  append_StringBuffer(       r(ss), "<oracle_start trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\"");

  if (strlen(subsystem) > 0) {
    append_StringBuffer(       r(ss), " package=\"");
    appendString_StringBuffer( r(ss), encodeAttValue(subsystem) );
    append_StringBuffer(       r(ss), "\"");
  }
  append_StringBuffer(       r(ss), " signature=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(signature) );
  append_StringBuffer(       r(ss), "\"");

  if (ref > 0) {
    append_StringBuffer(       r(ss), " ref=\"");
    appendFormat_StringBuffer( r(ss), "%ld", ref);
    append_StringBuffer(       r(ss), "\"");
  }
  append_StringBuffer(       r(ss), "/>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_tracePreconditionEnd(void *state, int traceId)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent << "<precondition_end" << " trace=\"" << traceId << "\"" << "/>" << endl;

  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<precondition_end trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\"/>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceOracleEnd(void *state, int traceId)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << --indent << "<oracle_end" << " trace=\"" << traceId << "\"" << "/>" << endl;

  st->indent.cur_indent -= st->indent.indent_size;
  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<oracle_end trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\"/>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceModelOperationEnd(void *state, int traceId)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << --indent << "<model_operation_end" << " trace=\"" << traceId << "\"" << "/>" << endl;

  st->indent.cur_indent -= st->indent.indent_size;
  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<model_operation_end trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\"/>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceSeriesStart(void *state, int traceId)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent++ << "<serialization_start" << " trace=\"" << traceId << "\"" << "/>" << endl;

  Indent_print(st->indent, ss);
  st->indent.cur_indent += st->indent.indent_size;
  append_StringBuffer(       r(ss), "<serialization_start trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\"/>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceSeriesEnd(void *state, int traceId)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << --indent << "<serialization_end" << " trace=\"" << traceId << "\"" << "/>" << endl;

  st->indent.cur_indent -= st->indent.indent_size;
  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<serialization_end trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\"/>\n");

  return toString(ss);
}


/*
 * Coverage level methods
 */

String * TraceXMLFormatter_tracePrimeFormula(void *state, int traceId, int id, TraceBool value)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent << "<prime_formula" << " trace=\"" << traceId << "\""
//  << " id=\"" << id << "\"" << " value=\"" << encodeBoolValue(value) << "\""<< "/>" << endl;

  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<prime_formula trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\" id=\"");
  appendFormat_StringBuffer( r(ss), "%d", id);
  append_StringBuffer(       r(ss), "\" value=\"");
  append_StringBuffer(       r(ss), encodeBoolValue(value) );
  append_StringBuffer(       r(ss), "\"/>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceCoverageElement(void *state, int traceId, const char *structureId, const char *coverageId, int branchId) {
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent << "<coverage_element" << " trace=\"" << traceId << "\""
//     << " coverage=\"" << encodeAttValue(coverageId) << "\"" << " id=\"" << branchId << "\"" << "/>" << endl;

  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<coverage_element trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\"");
  if( strlen( structureId ) != 0 )
  {
    append_StringBuffer(       r(ss), " structure=\"");
    appendString_StringBuffer( r(ss), encodeAttValue(structureId) );
    append_StringBuffer(       r(ss), "\"");
  }
  append_StringBuffer(       r(ss), " coverage=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(coverageId) );
  append_StringBuffer(       r(ss), "\" id=\"");
  appendFormat_StringBuffer( r(ss), "%d", branchId);
  append_StringBuffer(       r(ss), "\"/>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceMark(void *state, int traceId, const char *mark) {
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent << "<mark" << " trace=\"" << traceId << "\""
//     << " name=\"" << encodeAttValue(mark) << "\"" << "/>" << endl;

  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<mark trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\" name=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(mark) );
  append_StringBuffer(       r(ss), "\"/>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceCoverageStructureStart(void *state, int traceId, const char* name)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent++ << "<coverage_structure" << " trace=\"" << traceId << "\"" << ">" << endl;

  Indent_print(st->indent, ss);
  st->indent.cur_indent += st->indent.indent_size;
  append_StringBuffer(       r(ss), "<coverage_structure trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\"" );

  if( strlen( name ) != 0 )
  {
    append_StringBuffer( r(ss), " name=\"" );
    append_StringBuffer( r(ss), name );
    append_StringBuffer( r(ss), "\"" );
  }
  append_StringBuffer( r(ss), ">\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceFormulaeStart(void *state)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent++ << "<formulae>" << endl;

  Indent_print(st->indent, ss);
  st->indent.cur_indent += st->indent.indent_size;
  append_StringBuffer( r(ss), "<formulae>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceFormula(void *state, int id, const char *text)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent << "<formula" << " id=\"" << id << "\"" << ">" << "<![CDATA[" << text << "]]>" << "</formula>" << endl;

  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<formula id=\"");
  appendFormat_StringBuffer( r(ss), "%d", id);
  append_StringBuffer(       r(ss), "\"><![CDATA[");
  appendString_StringBuffer( r(ss), encodeCdataValue(text) );
  append_StringBuffer(       r(ss), "]]></formula>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceFormulaeEnd(void *state)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << --indent << "</formulae>" << endl;

  st->indent.cur_indent -= st->indent.indent_size;
  Indent_print(st->indent, ss);
  append_StringBuffer(r(ss), "</formulae>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceCoverageStart(void *state, const char *id)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent++ << "<coverage" << " id=\"" << encodeAttValue(id) << "\"" << ">" << endl;

  Indent_print(st->indent, ss);
  st->indent.cur_indent += st->indent.indent_size;
  append_StringBuffer(       r(ss), "<coverage id=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(id) );
  append_StringBuffer(       r(ss), "\">\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceElement(void *state, int id, const char *name)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent << "<element" << " id=\"" << id << "\"" << " name=\"" << encodeAttValue(name) << "\"" << "/>" << endl;

  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<element id=\"");
  appendFormat_StringBuffer( r(ss), "%d", id);
  append_StringBuffer(       r(ss), "\" name=\"");
  appendString_StringBuffer( r(ss), encodeAttValue(name) );
  append_StringBuffer(       r(ss), "\"/>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceCoverageEnd(void *state)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << --indent << "</coverage>" << endl;

  st->indent.cur_indent -= st->indent.indent_size;
  Indent_print(st->indent, ss);
  append_StringBuffer(r(ss), "</coverage>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceCoverageStructureEnd(void *state)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << --indent << "</coverage_structure>" << endl;

  st->indent.cur_indent -= st->indent.indent_size;
  Indent_print(st->indent, ss);
  append_StringBuffer(r(ss), "</coverage_structure>\n");

  return toString(ss);
}


/*
 * Other messages
 */
String * TraceXMLFormatter_traceSystemInfo(void *state, int traceId, const char *info);

// TODO: <where> ?
static String *format_exception(void *state, int traceId, const char *kind, const TraceList *values, const TraceList *infos, bool isInterim)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  TracePair *vl;
  TraceListitem *item;
  char buff[3*sizeof(int)+2];

  StringBuffer * ss = create_StringBuffer();
  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent++ << "<exception" << " trace=\"" << traceId << "\""
//  << " internal=\"false\"" << (isInterim ? " interim=\"true\"" : "") << ">" << endl;
  Indent_print(st->indent, ss);
  st->indent.cur_indent += st->indent.indent_size;
  sprintf(buff, "%d", traceId);

  append_StringBuffer( r(ss), "<exception trace=\"");
  append_StringBuffer( r(ss), buff);
  append_StringBuffer( r(ss), "\" kind=\"");
  append_StringBuffer( r(ss), kind);	// Hope, it does not need escaping
  append_StringBuffer( r(ss), "\" internal=\"false\"");
  if (isInterim)
	append_StringBuffer( r(ss), " interim=\"true\"");
  append_StringBuffer( r(ss), ">\n");

  // Trace exception values
  if(NULL != values) {
    for(item = values->head;(item != NULL); item = item->next)
    {
      vl = (TracePair *)item->data;
      if(!vl) continue;
// ss << indent << "<property name='"<< encodeAttValue(cur.first)
//	  << "' value='"<< encodeAttValue(cur.second) <<"' />" << endl;
      Indent_print(st->indent, ss);
      append_StringBuffer(       r(ss), "<property name=\"");
      appendString_StringBuffer( r(ss), encodeAttValue(vl->first));
      append_StringBuffer(       r(ss), "\" value=\"");
      appendString_StringBuffer( r(ss), encodeAttValue(vl->second));
      append_StringBuffer(       r(ss), "\"/>\n");
    }
  }

  // Trace exception infos
  if(NULL != infos) {
    for(item = infos->head;(item != NULL); item = item->next) {
      appendString_StringBuffer(r(ss), TraceXMLFormatter_traceSystemInfo(state,traceId,(char *)item->data));
    }
  }

  //  ss << --indent << "</exception>" << endl;
  st->indent.cur_indent -= st->indent.indent_size;
  Indent_print(st->indent, ss);
  append_StringBuffer(r(ss), "</exception>\n");

  return toString(ss);
}

String * TraceXMLFormatter_traceException(void *state, int traceId, const char *kind, const TraceList *values, const TraceList *infos)
{
	return format_exception(state, traceId, kind, values, infos, false);
}

String * TraceXMLFormatter_traceInterimException(void *state, int traceId, const char *kind, const TraceList *values, const TraceList *infos)
{
	return format_exception(state, traceId, kind, values, infos, true);
}

///////////////////////////////////
// TODO: <where> ?
String * TraceXMLFormatter_traceInternalError(void *state, int traceId, const char *info)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent++ << "<exception" << " trace=\"" << traceId << "\"" << " internal=\"true\"" << ">" << endl;
  Indent_print(st->indent, ss);
  st->indent.cur_indent += st->indent.indent_size;
  append_StringBuffer(       r(ss), "<exception trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\" internal=\"true\">\n");

//  ss << indent   << "<info><![CDATA[" << info << "]]></info>" << endl;
  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<info><![CDATA[");
  appendString_StringBuffer( r(ss), encodeCdataValue(info) );
  append_StringBuffer(       r(ss), "]]></info>\n");

//  ss << --indent << "</exception>" << endl;
  st->indent.cur_indent -= st->indent.indent_size;
  Indent_print(st->indent, ss);
  append_StringBuffer(r(ss), "</exception>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceSystemInfo(void *state, int traceId, const char *info)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent << "<info trace=\"" << traceId << "\">" << " <![CDATA[" << info << "]]>" << "</info>" << endl;
  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<info trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\"> <![CDATA[");
  appendString_StringBuffer( r(ss), encodeCdataValue(info) );
  append_StringBuffer(       r(ss), "]]></info>\n");

  return toString(ss);
}
///////////////////////////////////
String * TraceXMLFormatter_traceUserInfo(void *state, int traceId, const char *info)
{
  TraceXMLFormatter_state * st = (TraceXMLFormatter_state *)state;
  StringBuffer * ss = create_StringBuffer();

  if(st == NULL || ss == NULL)	return NULL;

//  ss << indent++ << "<user_info" << " trace=\"" << traceId << "\"" << ">" << endl;
  Indent_print(st->indent, ss);
  st->indent.cur_indent += st->indent.indent_size;
  append_StringBuffer(       r(ss), "<user_info trace=\"");
  appendFormat_StringBuffer( r(ss), "%d", traceId);
  append_StringBuffer(       r(ss), "\">\n");

// ss << indent   << "<![CDATA[" << info << "]]>" << endl;
  Indent_print(st->indent, ss);
  append_StringBuffer(       r(ss), "<![CDATA[");
  appendString_StringBuffer( r(ss), encodeCdataValue(info) );
  append_StringBuffer(       r(ss), "]]>\n");

// ss << --indent << "</user_info>" << endl;
  st->indent.cur_indent -= st->indent.indent_size;
  Indent_print(st->indent, ss);
  append_StringBuffer(r(ss), "</user_info>\n");

  return toString(ss);
}
///////////////////////////////////
TraceFormatter * TraceXMLFormatter_create(int indent_size, char indent_char)
{
TraceFormatter * tf;
TraceXMLFormatter_state * st;

  tf = (TraceFormatter *)malloc(sizeof(*tf));
  if(tf != NULL) {
    st = TraceXMLFormatter_state_create(indent_size, indent_char);
    if(st != NULL) {
      tf->state = st;
      tf->delete_state = TraceXMLFormatter_state_delete;

      tf->startTrace = TraceXMLFormatter_startTrace;
      tf->endTrace = TraceXMLFormatter_endTrace;

      tf->traceScenarioStart = TraceXMLFormatter_traceScenarioStart;
      tf->traceEnvironmentProperties = TraceXMLFormatter_traceEnvironmentProperties;
      tf->traceScenarioParameters = TraceXMLFormatter_traceScenarioParameters;
      tf->traceTestEngineProperty = TraceXMLFormatter_traceTestEngineProperty;
      tf->traceScenarioEnd = TraceXMLFormatter_traceScenarioEnd;

      tf->traceScenarioValue = TraceXMLFormatter_traceScenarioValue;
      tf->traceState = TraceXMLFormatter_traceState;
      tf->traceTransitionStart = TraceXMLFormatter_traceTransitionStart;
      tf->traceTransitionEnd = TraceXMLFormatter_traceTransitionEnd;

      tf->traceModelOperationStart = TraceXMLFormatter_traceModelOperationStart;
      tf->traceModelOperationArgument = TraceXMLFormatter_traceModelOperationArgument;
      tf->traceModelOperationResult = TraceXMLFormatter_traceModelOperationResult;
      tf->traceModelOperationChannel = TraceXMLFormatter_traceModelOperationChannel;
      tf->traceModelOperationTimestamp = TraceXMLFormatter_traceModelOperationTimestamp;
      tf->traceOracleStart = TraceXMLFormatter_traceOracleStart;
      tf->tracePreconditionEnd = TraceXMLFormatter_tracePreconditionEnd;
      tf->traceOracleEnd = TraceXMLFormatter_traceOracleEnd;
      tf->traceModelOperationEnd = TraceXMLFormatter_traceModelOperationEnd;
      tf->traceSeriesStart = TraceXMLFormatter_traceSeriesStart;
      tf->traceSeriesEnd = TraceXMLFormatter_traceSeriesEnd;

      tf->tracePrimeFormula = TraceXMLFormatter_tracePrimeFormula;
      tf->traceCoverageElement = TraceXMLFormatter_traceCoverageElement;
      tf->traceMark = TraceXMLFormatter_traceMark;
      tf->traceCoverageStructureStart = TraceXMLFormatter_traceCoverageStructureStart;
      tf->traceFormulaeStart = TraceXMLFormatter_traceFormulaeStart;
      tf->traceFormula = TraceXMLFormatter_traceFormula;
      tf->traceFormulaeEnd = TraceXMLFormatter_traceFormulaeEnd;
      tf->traceCoverageStart = TraceXMLFormatter_traceCoverageStart;
      tf->traceElement = TraceXMLFormatter_traceElement;
      tf->traceCoverageEnd = TraceXMLFormatter_traceCoverageEnd;
      tf->traceCoverageStructureEnd = TraceXMLFormatter_traceCoverageStructureEnd;

      tf->traceException = TraceXMLFormatter_traceException;
      tf->traceInterimException = TraceXMLFormatter_traceInterimException;
      tf->traceInternalError = TraceXMLFormatter_traceInternalError;
      tf->traceSystemInfo = TraceXMLFormatter_traceSystemInfo;
      tf->traceUserInfo = TraceXMLFormatter_traceUserInfo;
      tf->valid = TraceXMLFormatter_valid;
    }
    else { free(tf); tf = NULL; }
  }
  return tf;
}

