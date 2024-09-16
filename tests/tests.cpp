#include <simplistic/di.h>
#include <cassert>

using namespace simplistic::di;

template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, bool> Compare(T a, T b, T epsilon = 1e-6) {
	return std::fabs(a - b) < epsilon;
}

void LocalScopeBoundTest()
{
	Container c;

	c.Install(0.f);

	{
		ContainerScope s1(&c);

		s1.Install(1.f);

		{
			ContainerScope s2(&s1);

			s2.Install(2.f);

			{
				ContainerScope s3(&s2);

				s3.Install(3.f);

				assert(Compare(s3.GetO<float>(), 3.f));
			}

			assert(Compare(s2.GetO<float>(), 2.f));
		}

		assert(Compare(s1.GetO<float>(), 1.f));
	}

	assert(Compare(c.GetO<float>(), 0.f));

	printf("LocalScopeBoundTest Succesfully\n");
}

void FarScopeAccessTest()
{
	Container c;

	c.Install(1.f);

	{
		ContainerScope s1(&c);

		{
			ContainerScope s2(&s1);

			{
				ContainerScope s3(&s2);

				s3.Install(3.f);

				assert(Compare(s3.GetO<float>(), 3.f));
			}

			assert(Compare(s2.GetO<float>(), 1.f));
		}

		assert(Compare(s1.GetO<float>(), 1.f));
	}

	assert(Compare(c.GetO<float>(), 1.f));

	printf("FarScopeAccessTest Succesfully\n");
}

int main()
{
	LocalScopeBoundTest();
	FarScopeAccessTest();
}