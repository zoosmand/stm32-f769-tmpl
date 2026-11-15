#ifndef LOG_BUFFER_H
#define LOG_BUFFER_H

#include <stddef.h>
#include <stdint.h>

/** Characters per stored line; longer output wraps onto the next line. */
#define LOG_BUFFER_COLUMNS  66U
/** Lines retained, including the line currently being written. */
#define LOG_BUFFER_LINES    64U

/**
  * @brief Append text to the line ring buffer.
  * @param text (const char*) Characters to append; may be null when length is 0.
  * @param length (size_t) Number of characters.
  * @note  Called from _write(), so it captures all printf() output. '\n' ends a
  *        line, '\r' is ignored, '\b' moves the cursor left, other control
  *        characters are dropped, and the oldest line is overwritten when full.
  *        Thread/main-loop context only; not safe to call from an interrupt.
  */
void LogBuffer_Write(const char *text, size_t length);

/**
  * @brief Return one stored line.
  * @param ageLines (uint32_t) 0 for the line being written, 1 for the previous
  *        complete line, and so on up to LogBuffer_GetLineCount() - 1.
  * @param length (uint16_t*) Non-null; receives the line length in characters.
  * @retval (const char*) Pointer to the line (not NUL-terminated), owned by this
  *         module and valid until the next LogBuffer_Write(); NULL when ageLines
  *         is out of range.
  */
const char *LogBuffer_GetLine(uint32_t ageLines, uint16_t *length);

/** @brief Number of retrievable lines (1..LOG_BUFFER_LINES). */
uint32_t LogBuffer_GetLineCount(void);

/** @brief Counter that changes whenever the buffer content changes. */
uint32_t LogBuffer_GetSequence(void);

#endif /* LOG_BUFFER_H */
