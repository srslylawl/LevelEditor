#pragma once
namespace Rendering {
	class Renderable {
	public:
		bool renderingEnabled = true;
		virtual ~Renderable() = default;
		virtual void Render() const = 0;
	};
}


