/*
 * Copyright © 2012 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by:
 *   Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "mir/graphics/android/android_buffer_handle.h"

namespace mga=mir::graphics::android;
namespace mc=mir::compositor;
namespace geom=mir::geometry;

mga::AndroidBufferHandle::AndroidBufferHandle(buffer_handle_t han, geom::Width w, geom::Height h,
                                              geom::Stride s, int pf, int use)
{
    anw_buffer.height =(int) h.as_uint32_t();
    anw_buffer.width = (int) w.as_uint32_t();
    anw_buffer.handle = han;
    anw_buffer.stride = (int) s.as_uint32_t();
    anw_buffer.usage = use;
    anw_buffer.format = pf;
}

EGLClientBuffer mga::AndroidBufferHandle::get_egl_client_buffer()
{
    return (EGLClientBuffer) NULL;
}

geom::Height mga::AndroidBufferHandle::height()
{
    return geom::Height(anw_buffer.height);
}

geom::Width mga::AndroidBufferHandle::width()
{
    return geom::Width(anw_buffer.width);
}

geom::Stride mga::AndroidBufferHandle::stride()
{
    return geom::Stride(anw_buffer.stride);
}

mc::PixelFormat mga::AndroidBufferHandle::format()
{
    return mc::PixelFormat::rgba_8888;
}

mga::BufferUsage mga::AndroidBufferHandle::usage()
{
    return mga::BufferUsage::use_hardware;
}
