#pragma once

namespace CG
{
    class WaterTile {
        public:
            static constexpr float TILE_SIZE = 10.0f;

            WaterTile(float centerX, float centerZ, float height)
                : x(centerX), z(centerZ), height(height) {}

            float getHeight() const { return height; }
            float getX() const { return x; }
            float getZ() const { return z; }

        private:
            float height;
            float x;
            float z;
    };
}
