#pragma once

#include <utility>
#include <random>

namespace td
{
	// A noise abstract texture function
	class noise
	{
	public:
		virtual float sample(float x, float y) const = 0;
	};



	class perlin_noise : public noise
	{
	public:
		perlin_noise(unsigned int seed = 0)
			: m_seed(seed)
		{}

		float sample(float x, float y) const override
		{
			if (x < 0 || y < 0)
				throw std::runtime_error("cannot sample at a negative coordinate");

			float fx = floor(x);
			float fy = floor(y);

			unsigned int ix = fx;
			unsigned int iy = fy;


			// sample four corners
			using vec2 = std::pair<float, float>;

			vec2 bl = gradient(ix, iy);
			vec2 br = gradient(ix + 1, iy);
			vec2 tl = gradient(ix, iy + 1);
			vec2 tr = gradient(ix + 1, iy + 1);

			// compute four dot products
			float dbl = ((fx - x) * bl.first) + ((fy - y) * bl.second);
			float dbr = ((fx + 1 - x) * br.first) + ((fy - y) * br.second);
			float dtl = ((fx - x) * tl.first) + ((fy + 1 - y) * tl.second);
			float dtr = ((fx + 1 - x) * tr.first) + ((fy + 1 - y) * tr.second);

			// smoothstep interpolate
			// st(x) = 3x^2 - 2x^3
			auto st = [](float x)
			{
				return 3 * x * x - 2 * x * x * x;
			};

			float dx = x - fx;
			float dy = y - fy;
			float top = dtl + st(dx) * (dtr - dtl);
			float bottom = dbl + st(dx) * (dbr - dbl);

			return bottom + st(dy) * (top - bottom);
		}

	private:
		std::pair<float, float> gradient(unsigned int x, unsigned int y) const
		{
			x += m_seed;
			y += m_seed;
			const unsigned w = 8 * sizeof(unsigned);
			const unsigned s = w / 2; // rotation width
			unsigned a = x, b = y;
			a *= 3284157443; b ^= a << s | a >> w - s;
			b *= 1911520717; a ^= b << s | b >> w - s;
			a *= 2048419325;
			float phase = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]

			return { cos(phase), sin(phase) };
		}

	private:
		unsigned int m_seed;
	};




}