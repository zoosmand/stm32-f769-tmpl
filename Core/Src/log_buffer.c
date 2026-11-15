#include "log_buffer.h"

/* LOG_BUFFER_LINES must be a power of two for the index mask. */
#if (LOG_BUFFER_LINES & (LOG_BUFFER_LINES - 1U)) != 0U
#error "LOG_BUFFER_LINES must be a power of two"
#endif

static char logLines[LOG_BUFFER_LINES][LOG_BUFFER_COLUMNS];
static uint16_t logLengths[LOG_BUFFER_LINES];
static uint32_t logLinesStarted = 1U;   /* Line 0 is open from the start. */
static uint16_t logColumn;
static uint32_t logSequence;

static uint32_t logBuffer_CurrentIndex(void)
{
  return (logLinesStarted - 1U) & (LOG_BUFFER_LINES - 1U);
}

static void logBuffer_NewLine(void)
{
  logLinesStarted++;
  logLengths[logBuffer_CurrentIndex()] = 0U;
  logColumn = 0U;
}

void LogBuffer_Write(const char *text, size_t length)
{
  if ((text == NULL) || (length == 0U))
  {
    return;
  }

  for (size_t index = 0U; index < length; index++)
  {
    char ch = text[index];
    if (ch == '\n')
    {
      logBuffer_NewLine();
      continue;
    }
    if (ch == '\b')
    {
      if (logColumn > 0U)
      {
        logColumn--;
      }
      continue;
    }
    if (ch == '\t')
    {
      ch = ' ';
    }
    if ((ch < ' ') || (ch > '~'))
    {
      continue;   /* '\r' and other control characters. */
    }

    if (logColumn >= LOG_BUFFER_COLUMNS)
    {
      logBuffer_NewLine();
    }
    const uint32_t line = logBuffer_CurrentIndex();
    logLines[line][logColumn++] = ch;
    if (logColumn > logLengths[line])
    {
      logLengths[line] = logColumn;
    }
  }
  logSequence++;
}

const char *LogBuffer_GetLine(uint32_t ageLines, uint16_t *length)
{
  if ((length == NULL) || (ageLines >= LogBuffer_GetLineCount()))
  {
    return NULL;
  }

  const uint32_t line = (logLinesStarted - 1U - ageLines) &
                        (LOG_BUFFER_LINES - 1U);
  *length = logLengths[line];
  return logLines[line];
}

uint32_t LogBuffer_GetLineCount(void)
{
  return (logLinesStarted < LOG_BUFFER_LINES) ? logLinesStarted :
                                                LOG_BUFFER_LINES;
}

uint32_t LogBuffer_GetSequence(void)
{
  return logSequence;
}
