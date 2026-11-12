#pragma once
#include "MathCore.hpp"
#include "MoonCore/Containers/String.hpp"
#include "MoonCore/Utility/Tuple.hpp"
#include "MathCore.hpp"

namespace MC::Math
{
    struct Color3
    {
    public:
        union
        {
            struct { u8 r, g, b; };
            struct { u8 R, G, B; };
            u8 RGB[3];
        };

    public:
        constexpr Color3() noexcept 
            : R(0), G(0), B(0) 
        {}
        
        constexpr Color3(u8 red, u8 green, u8 blue) noexcept
            : R(red), G(green), B(blue) 
        {}

        MC::String ToHex() const noexcept
        {
            return MC::Utility::ConvertNumberToHex(R) + MC::Utility::ConvertNumberToHex(G) + MC::Utility::ConvertNumberToHex(B);
        }
        constexpr MC::Utility::Tuple<f32, f32, f32> ToHSV() const noexcept
        {
            f32 rf = static_cast<f32>(R) / 255, 
                gf = static_cast<f32>(G) / 255, 
                bf = static_cast<f32>(B) / 255;

            f32 max = Max(rf, Max(gf, bf));
            f32 min = Min(rf, Min(gf, bf));
            f32 delta = max - min;

            f32 H;
            if (delta <= Epsilon<f32>) H = 0.f;
            else if (rf >= gf && rf >= bf)
            {
                H = 60 * ((gf - bf) / delta);
                if (H < 0) H += 360;
            }
            else if (gf > rf && gf > bf)
                H = 60 * ((bf - rf) / delta + 2);
            else
                H = 60 * ((rf - gf) / delta + 4);

            f32 S = Epsilon<f32> <= max && max <= Epsilon<f32> ? 0.f : delta / max;

            f32 V = max;
            return { H, S, V };
        }
        constexpr Color3 Lerp(Color3 target, f32 alpha) const noexcept
        {
            return { 
                Math::Lerp(R, target.R, alpha), 
                Math::Lerp(G, target.G, alpha), 
                Math::Lerp(B, target.B, alpha)
            };
        }

        constexpr Vector3D ToVector() const noexcept
        {
            return Vector3D(static_cast<f32>(r) / 255.f, static_cast<f32>(g) / 255.f, static_cast<f32>(b) / 255.f);
        }

    public:
        static Color3 FromHSV(f32 hue, f32 saturation, f32 value) noexcept
        {
            hue = Clamp<f32>(0, 360, hue);
            saturation = Clamp<f32>(0, 1, saturation);
            value = Clamp<f32>(0, 1, value);
            f32 C = value * saturation;
            f32 X = C * (1 - fabsf(fmodf(hue / 60, 2.f) - 1));
            f32 m = value - C;

            f32 rf;
            f32 gf;
            f32 bf;

            if (0.f <= hue && hue < 60.f)
            {
                rf = C;
                gf = X;
                bf = 0;
            }
            else if (60.f <= hue && hue < 120.f)
            {
                rf = X;
                gf = C;
                bf = 0;
            }
            else if (120.f <= hue && hue < 180.f)
            {
                rf = 0;
                gf = C;
                bf = X;
            }
            else if (180.f <= hue && hue < 240.f)
            {
                rf = 0;
                gf = X;
                bf = C;
            }
            else if (240.f <= hue && hue < 300.f)
            {
                rf = X;
                gf = 0;
                bf = C;
            }
            else
            {
                rf = C;
                gf = 0;
                bf = X;
            }

            return {
                static_cast<u8>((rf + m) * 255),
                static_cast<u8>((rf + m) * 255),
                static_cast<u8>((rf + m) * 255) 
            };
        }
        static constexpr Color3 FromHex(const MC::StringView& hex)
        {
            if (hex.Size() != 6) return {};
            return {
                static_cast<u8>(Utility::ConvertHexToNumber(hex.Substring(0, 2))),
                static_cast<u8>(Utility::ConvertHexToNumber(hex.Substring(2, 2))),
                static_cast<u8>(Utility::ConvertHexToNumber(hex.Substring(4, 2))),
            };
        }
    };
    
    struct Color4
    {
    public:
        union 
        {
            struct { u8 r, g, b, a; };
            struct { u8 R, G, B, A; };
            u8 RGBA[4];
        };

    public:
        constexpr Color4() noexcept : R(0), G(0), B(0), A(255) {}
        constexpr Color4(u8 rValue, u8 gValue, u8 bValue, u8 aValue) noexcept : R(rValue), G(gValue), B(bValue), A(aValue) {}
        constexpr Color4(u32 rgba) noexcept
        {
            R = static_cast<u8>(rgba >> 24);
            G = static_cast<u8>((rgba & 0x00FF0000) >> 16);
            B = static_cast<u8>((rgba & 0x0000FF00) >> 8);
            A = static_cast<u8>(rgba & 0xFF);
        }
        constexpr Color4 Lerp(Color4 target, f32 t) const noexcept
        {
            Color4 result;
            result.R = Math::Lerp(R, target.R, t);
            result.G = Math::Lerp(G, target.G, t);
            result.B = Math::Lerp(B, target.B, t);
            result.A = Math::Lerp(A, target.A, t);
            return result;
        }

        constexpr Vector4D AsVector() const noexcept
        {
            return Vector4D(static_cast<f32>(r) / 255.f, static_cast<f32>(g) / 255.f, static_cast<f32>(b) / 255.f, static_cast<f32>(a) / 255.f);
        }
    };
}