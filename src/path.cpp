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
#include <peelo/path.hpp>
#include <peelo/unicode.hpp>

#if defined(_WIN32)
# define UNICODE
# include <windows.h>
#endif

namespace peelo
{
#if defined(_WIN32)
  const char32_t path::separator = '\\';
#else
  const char32_t path::separator = '/';
#endif

#if defined(_WIN32)
  static int compare_ignore_case(const std::u32string&, const std::u32string&);
  static std::wstring widen(const std::u32string&);
#endif
  static void parse(
    const std::u32string&,
    std::u32string&,
    std::u32string&,
    std::vector<std::u32string>&
  );

  bool
  path::is_separator(char32_t c)
  {
    return c == '/' || c == '\\';
  }

  path::path()
#if !defined(_WIN32)
    : m_stat_state(stat_state_uninitialized)
#endif
    {}

  path::path(const path& that)
    : m_full_path(that.m_full_path)
    , m_root(that.m_root)
    , m_parts(that.m_parts)
#if !defined(_WIN32)
    , m_stat_state(that.m_stat_state)
    , m_stat(that.m_stat)
#endif
    {}

  path::path(const std::u32string& source)
#if !defined(_WIN32)
    : m_stat_state(stat_state_uninitialized)
#endif
  {
    parse(source, m_full_path, m_root, m_parts);
  }

  void
  path::assign(const path& that)
  {
    m_full_path = that.m_full_path;
    m_root = that.m_root;
    m_parts = that.m_parts;
  }

  void
  path::assign(const std::u32string& source)
  {
    m_full_path.clear();
    m_root.clear();
    m_parts.clear();
    parse(source, m_full_path, m_root, m_parts);
  }

  bool
  path::exists() const
  {
    if (empty())
    {
      return false;
    }

#if defined(_WIN32)
    return ::PathFileExists(widen(m_full_path).c_str());
#else
    return stat();
#endif
  }

  bool
  path::is_dir() const
  {
    if (empty())
    {
      return false;
    }

#if defined(_WIN32)
    return ::GetFileAttributesW(widen(m_full_path).c_str()) &
      FILE_ATTRIBUTE_DIRECTORY;
#else
    return stat() && S_ISDIR(m_stat.st_mode);
#endif
  }

  bool
  path::is_file() const
  {
    if (empty())
    {
      return false;
    }

#if defined(_WIN32)
    return ::GetFileAttributesW(widen(m_full_path).c_str()) &
      FILE_ATTRIBUTE_NORMAL;
#else
    return stat() && S_ISREG(m_stat.st_mode);
#endif
  }

  bool
  path::is_symlink() const
  {
#if !defined(_WIN32) && defined(S_ISLNK)
    if (!empty())
    {
      return stat() && S_ISLNK(m_stat.st_mode);
    }
#endif

    return false;
  }

  bool
  path::is_socket() const
  {
#if !defined(_WIN32) && defined(S_ISSOCK)
    if (!empty())
    {
      return stat() && S_ISSOCK(m_stat.st_mode);
    }
#endif

    return false;
  }

  bool
  path::is_fifo() const
  {
#if !defined(_WIN32) && defined(S_ISFIFO)
    if (!empty())
    {
      return stat() && S_ISFIFO(m_stat.st_mode);
    }
#endif

    return false;
  }

  bool
  path::is_char_device() const
  {
#if !defined(_WIN32) && defined(S_ISCHR)
    if (!empty())
    {
      return stat() && S_ISCHR(m_stat.st_mode);
    }
#endif

    return false;
  }

  bool
  path::is_block_device() const
  {
#if !defined(_WIN32) && defined(S_ISBLK)
    if (empty())
    {
      return stat() && S_ISBLK(m_stat.st_mode);
    }
#endif

    return false;
  }

  std::optional<std::size_t>
  path::size() const
  {
    if (!empty())
    {
#if defined(_WIN32)
      WIN32_FILE_ATTRIBUTE_DATA data = {0};

      if (::GetFileAttributesExW(widen(m_full_path).c_str(),
                                 GetFileExInfoStandard,
                                 &data))
      {
        const auto size = (
          (static_cast<ULONGLONG>(data.nFileSizeHigh) <<
           sizeof(data.nFileSizeLow) * 8) | data.nFileSizeLow
        );

        return std::optional<std::size_t>(static_cast<std::size_t>(size));
      }
#else
      if (stat())
      {
        return std::optional<std::size_t>(m_stat.st_size);
      }
#endif
    }

    return std::optional<std::size_t>();
  }

  std::optional<datetime>
  path::last_access() const
  {
#if defined(_WIN32)
    WIN32_FILE_ATTRIBUTE_DATA data = {0};

    if (::GetFileAttributesExW(widen(m_full_path).c_str(),
                               GetFileExInfoStandard,
                               &data))
    {
      SYSTEMTIME st;

      ::FileTimeToSystemTime(&data.ftLastAccessTime, &st);

      return std::make_optional<datetime>(
        st.wYear,
        static_cast<month>(st.wMonth - 1),
        st.wDay,
        st.wHour,
        st.wMinute,
        st.wSecond
      );
    }
#else
    if (stat())
    {
      return std::optional<datetime>(datetime::timestamp(m_stat.st_atime));
    }
#endif

    return std::optional<datetime>();
  }

  std::optional<datetime>
  path::last_modified() const
  {
#if defined(_WIN32)
    WIN32_FILE_ATTRIBUTE_DATA data = {0};

    if (::GetFileAttributesExW(widen(m_full_path).c_str(),
                               GetFileExInfoStandard,
                               &data))
    {
      SYSTEMTIME st;

      ::FileTimeToSystemTime(&data.ftLastWriteTime, &st);

      return std::make_optional<datetime>(
        st.wYear,
        static_cast<month>(st.wMonth - 1),
        st.wDay,
        st.wHour,
        st.wMinute,
        st.wSecond
      );
    }
#else
    if (stat())
    {
      return std::optional<datetime>(datetime::timestamp(m_stat.st_mtime));
    }
#endif

    return std::optional<datetime>();
  }

  bool
  path::equals(const path& that) const
  {
    if (empty())
    {
      return that.empty();
    }

#if defined(_WIN32)
    return !compare_ignore_case(m_full_path, that.m_full_path);
#else
    return !m_full_path.compare(that.m_full_path);
#endif
  }

  int
  path::compare(const path& that) const
  {
    if (empty())
    {
      return that.empty() ? 0 : -1;
    }

#if defined(_WIN32)
    return compare_ignore_case(m_full_path, that.m_full_path);
#else
    return m_full_path.compare(that.m_full_path);
#endif
  }

#if !defined(_WIN32)
  bool
  path::stat() const
  {
    if (m_stat_state == stat_state_failure)
    {
      return false;
    }
    else if (m_stat_state == stat_state_uninitialized)
    {
      const auto encoded_path = unicode::utf8::encode(m_full_path);

      if (::stat(encoded_path.c_str(), &m_stat) < 0)
      {
        m_stat_state = stat_state_failure;

        return false;
      }
      m_stat_state = stat_state_success;
    }

    return true;
  }
#endif

#if defined(_WIN32)
  static int
  compare_ignore_case(const std::u32string& a, const std::u32string& b)
  {
    const auto length_a = a.length();
    const auto length_b = b.length();
    const auto n = std::min(length_a, length_b);

    for (std::u32string::size_type i = 0; i < n; ++i)
    {
      const auto c1 = unicode::tolower(a[i]);
      const auto c2 = unicode::tolower(b[i]);

      if (c1 != c2)
      {
        return c1 > c2 ? 1 : -1;
      }
    }
    if (length_a == length_b)
    {
      return 0;
    }

    return length_a > length_b ? 1 : -1;
  }

  static std::wstring
  widen(const std::u32string& input)
  {
    const auto length = input.length();
    std::wstring result;

    result.reserve(length);
    for (std::u32string::size_type i = 0; i < length; ++i)
    {
      const auto c = input[i];

      if (c > 0xffff)
      {
        result.append(1, static_cast<wchar_t>(c >> 16));
        result.append(1, static_cast<wchar_t>((c & 0xff00) >> 8));
      } else {
        result.append(1, static_cast<wchar_t>(c));
      }
    }

    return result;
  }
#endif

  static void
  append_part(const std::u32string& input, std::vector<std::u32string>& parts)
  {
    const auto length = input.length();

    if (!length)
    {
      return;
    }
    else if (input[0] == '.')
    {
      if (length == 2 && input[1] == '.')
      {
        if (!parts.empty())
        {
          parts.pop_back();
          return;
        }
      }
      else if (length == 1)
      {
        if (!parts.empty())
        {
          return;
        }
      }
    }
    parts.push_back(input);
  }

  static std::u32string
  compile_path(const std::u32string& root,
               const std::vector<std::u32string>& parts)
  {
    std::u32string result;

    if (!root.empty())
    {
      result.append(root);
    }
    for (std::size_t i = 0; i < parts.size(); ++i)
    {
      if (i > 0 && !path::is_separator(result.back()))
      {
        result.append(1, path::separator);
      }
      result.append(parts[i]);
    }

    return result;
  }

  static void
  parse(const std::u32string& source,
        std::u32string& full_path,
        std::u32string& root,
        std::vector<std::u32string>& parts)
  {
    const auto length = source.length();
    std::u32string::size_type begin = 0;
    std::u32string::size_type end = 0;

    if (!length)
    {
      return;
    }
    else if (path::is_separator(source[0]))
    {
      root = source.substr(0, 1);
      if (length == 1)
      {
        full_path = root;
        return;
      }
      begin = 1;
    }
#if defined(_WIN32)
    // Process drive letter on Windows platform.
    else if (length > 1 && std::isalpha(source[0]) && source[1] == ':')
    {
      if (length == 2)
      {
        full_path = root = source;
        return;
      }
      else if (path::is_separator(source[2]))
      {
        root = source.substr(0, 2);
        begin = 3;
      }
    }
#endif

    for (std::u32string::size_type i = begin; i < length; ++i)
    {
      if (path::is_separator(source[i]))
      {
        if (end > 0)
        {
          append_part(source.substr(begin, end), parts);
        }
        begin = i + 1;
        end = 0;
      } else {
        ++end;
      }
    }
    if (end > 0)
    {
      append_part(source.substr(begin), parts);
    }
    full_path = compile_path(root, parts);
  }
}
