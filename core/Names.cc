#include "Names.h"
#include "Context.h"
#include "Hashing.h"
#include <numeric> // accumulate

using namespace std;

namespace ruby_typer {
namespace core {

ruby_typer::core::Name::~Name() noexcept {
    if (kind == NameKind::UNIQUE)
        unique.~UniqueName();
}

unsigned int Name::hashNames(vector<NameRef> &lhs, GlobalState &gs) {
    return accumulate(lhs.begin(), lhs.end(), 0, [&gs](int acc, NameRef &necc) -> int { return mix(acc, necc.id()); }) *
               8 +
           lhs.size();
}

unsigned int Name::hash(GlobalState &gs) const {
    // TODO: use https://github.com/Cyan4973/xxHash
    // !!! keep this in sync with GlobalState.enter*
    switch (kind) {
        case UTF8:
            return _hash(raw.utf8);
        case UNIQUE: {
            return _hash_mix_unique((u2)unique.uniqueNameKind, UNIQUE, unique.num, unique.original.id());
        }
            DEBUG_ONLY(default : Error::raise("Unknown name kind?", kind);)
    }
}

string Name::toString(GlobalState &gs) const {
    if (kind == UTF8) {
        return raw.utf8.toString();
    } else if (kind == UNIQUE) {
        if (this->unique.uniqueNameKind == UniqueNameKind::Singleton) {
            return "<singleton class:" + this->unique.original.name(gs).toString(gs) + ">";
        }
        return this->unique.original.name(gs).toString(gs) + "$" + to_string(this->unique.num);
    } else {
        Error::notImplemented();
    }
}

Name &NameRef::name(GlobalState &gs) const {
    DEBUG_ONLY(Error::check(_id < gs.names.size()));
    DEBUG_ONLY(Error::check(exists()));
    return gs.names[_id];
}
string NameRef::toString(GlobalState &gs) const {
    return name(gs).toString(gs);
}

NameRef NameRef::addEq(GlobalState &gs) const {
    Name &name = this->name(gs);
    Error::check(name.kind == UTF8);
    string nameEq = name.raw.utf8.toString() + "=";
    return gs.enterNameUTF8(nameEq);
}

} // namespace core
} // namespace ruby_typer
