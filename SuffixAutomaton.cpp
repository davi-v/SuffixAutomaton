#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <string>

#ifdef _DEBUG
#define DEBUG_ASSERT(x) assert(x)
#else
#define DEBUG_ASSERT(x)
#endif

using namespace std;

class SuffixAutomaton
{
	struct State
	{
		size_t len, link;
		map<char, size_t> next;
	};
	unique_ptr<State[]> st;
	size_t sz;
public:
	template<class C>
	SuffixAutomaton(const C& s) :
		SuffixAutomaton(s.data(), s.size())
	{

	}
	SuffixAutomaton(const char* s, size_t n) :
		sz(1)
	{
		DEBUG_ASSERT(n);

		size_t last = 0;

		st = make_unique<State[]>(n * 2);
		st[0].len = 0;

		for (size_t i = 0; i != n; i++)
		{
			const auto& c = s[i];
			const auto cur = sz++;
			auto& vCur = st[cur];
			vCur.len = st[last].len + 1;
			auto& p = last;
			while (true)
			{
				auto [it, b] = st[p].next.emplace(c, cur);
				if (!b)
				{
					auto& q = it->second;
					auto& vQ = st[q];
					const auto lNxt = st[p].len + 1;
					if (lNxt == vQ.len)
						vCur.link = q;
					else
					{
						const auto clone = sz++;
						auto& vClone = st[clone];
						vClone.len = lNxt;
						vClone.next = vQ.next;
						vClone.link = vQ.link;
						vQ.link = vCur.link = clone;
						auto qPtr = &q;
						while (true)
						{
							*qPtr = clone;
							p = st[p].link;
							if (!p)
								break;
							qPtr = &st[p].next.at(c);
							if (*qPtr != q)
								break;
						}
					}
					break;
				}
				if (!p)
				{
					vCur.link = 0;
					break;
				}
				p = st[p].link;
			}
			last = cur;
		}
	}

	template<class C>
	bool has(const C& p)
	{
		return has(p.data(), p.size());
	}
	bool has(const char* p, size_t n)
	{
		for (size_t i = 0, cur = 0; i != n; i++)
		{
			const auto& v = st[cur];
			const auto& c = p[i];
			const auto& m = v.next;
			auto it = m.find(c);
			if (it == m.end())
				return false;
			cur = it->second;
		}
		return true;
	}
};

template<class T>
auto pi(const T& p)
{
	const auto n = p.size();
	vector<size_t> r(n);
	for (size_t i = 1, j = 0; i < n; i++)
	{
		while (j && p[j] != p[i])
			j = r[j - 1];
		if (p[j] == p[i])
			j++;
		r[i] = j;
	}
	return r;
}

template<class T>
auto KMPFindIndices(const T& t, const T& p)
{
	vector<size_t> r;
	auto v = pi(p);
	const auto textLen = t.size();
	for (size_t i = 0, j = 0; i != textLen; i++)
	{
		while (j && p[j] != t[i])
			j = v[j - 1];
		if (p[j] == t[i])
			j++;
		if (j == p.size())
		{
			r.emplace_back(i - j + 1);
			j = v[j - 1];
		}
	}
	return r;
}

int main()
{
	constexpr unsigned SEED = 24;
	constexpr unsigned T = 10;
	constexpr unsigned P = 100;
	mt19937 mt(SEED);
	uniform_int_distribution<size_t> distN(1, 1000000);
	uniform_int_distribution<size_t> distP(1, 1000);
	uniform_int_distribution<> distC('a', 'z');
	for (unsigned i = 0; i != T; i++)
	{
		const auto n = distN(mt);
		vector<char> s(n);
		for (auto& c : s)
			c = distC(mt);
		SuffixAutomaton suf(s);
		for (unsigned j = 0; j != P; j++)
		{
			const auto l = distP(mt);
			vector<char> p(l);
			for (auto& c : p)
				c = distC(mt);
			assert(!KMPFindIndices(s, p).empty() == suf.has(p));
		}
	}
	cout << "All tests passed\n";
}