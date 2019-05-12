/*
 * Copyright (c) 2019, peelo.net
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef PEELO_PATH_HPP_GUARD
#define PEELO_PATH_HPP_GUARD

#include <string>
#include <vector>

#if !defined(_WIN32)
# include <sys/stat.h>
#endif

namespace peelo
{
  class path
  {
  public:
    /** Platform specific path separator. */
    static const char32_t separator;

    /**
     * Returns true if given character is a path separator.
     */
    static bool is_separator(char32_t c);

    /**
     * Constructs empty path.
     */
    explicit path();

    /**
     * Constructs copy of existing path.
     *
     * \param that Path to create copy of.
     */
    path(const path& that);

    /**
     * Parses given string into a path.
     *
     * \param source String containing the path.
     */
    explicit path(const std::u32string& source);

    /**
     * Copies contents of another path into this one.
     *
     * \param that Other path to copy contents of.
     */
    inline path& operator=(const path& that)
    {
      assign(that);

      return *this;
    }

    /**
     * Replaces contents of the path with a path parsed from given string.
     *
     * \param source String to parse the new path from.
     */
    inline path& operator=(const std::u32string& source)
    {
      assign(source);

      return *this;
    }

    /**
     * Copies contents of another path into this one.
     *
     * \param that Other path to copy contents of.
     */
    void assign(const path& that);

    /**
     * Replaces contents of the path with a path parsed from given string.
     *
     * \param source String to parse the new path from.
     */
    void assign(const std::u32string& source);

    /**
     * Tests whether the path is empty.
     */
    inline bool empty() const
    {
      return m_full_path.empty();
    }

    /**
     * Tests whether the path exists on the file system.
     */
    bool exists() const;

    /**
     * Returns true if the path exists on the file system and is pointing to a
     * directory.
     */
    bool is_dir() const;

    /**
     * Returns true if the path exists on the file system and is pointing to a
     * regular file.
     */
    bool is_file() const;

    /**
     * Returns true if the path exists on the file system and is pointing to a
     * symbolic link.
     */
    bool is_symlink() const;

    /**
     * Returns true if the path exists on the file system and is pointing to a
     * Unix socket.
     */
    bool is_socket() const;

    /**
     * Returns true if the path exists on the file system and is pointing to a
     * FIFO.
     */
    bool is_fifo() const;

    /**
     * Returns true if the path exists on the file system and is pointing to a
     * character device.
     */
    bool is_char_device() const;

    /**
     * Returns true if the path exists on the file system and is pointing to a
     * block device.
     */
    bool is_block_device() const;

    /**
     * Tests whether two paths are equal.
     */
    bool equals(const path& that) const;

    /**
     * Equality testing operator.
     */
    inline bool operator==(const path& that) const
    {
      return equals(that);
    }

    /**
     * Non-equality testing operator.
     */
    inline bool operator!=(const path& that) const
    {
      return !equals(that);
    }

    /**
     * Compares two paths lexicographically against each other.
     */
    int compare(const path& that) const;

    /**
     * Comparison operator.
     */
    inline bool operator<(const path& that) const
    {
      return compare(that) < 0;
    }

    /**
     * Comparison operator.
     */
    inline bool operator>(const path& that) const
    {
      return compare(that) > 0;
    }

    /**
     * Comparison operator.
     */
    inline bool operator<=(const path& that) const
    {
      return compare(that) <= 0;
    }

    /**
     * Comparison operator.
     */
    inline bool operator>=(const path& that) const
    {
      return compare(that) >= 0;
    }

#if !defined(_WIN32)
  private:
    bool stat() const;
#endif

  private:
    std::u32string m_full_path;
    std::u32string m_root;
    std::vector<std::u32string> m_parts;
#if !defined(_WIN32)
    mutable enum
    {
      stat_state_uninitialized,
      stat_state_success,
      stat_state_failure
    } m_stat_state;
    mutable struct ::stat m_stat;
#endif
  };
}

#endif /* !PEELO_PATH_HPP_GUARD */
