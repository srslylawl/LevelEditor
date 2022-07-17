#pragma once
namespace Rendering {
	class Renderable {
	public:
		virtual ~Renderable() = default;
		virtual void Render() const = 0;
	};
}


