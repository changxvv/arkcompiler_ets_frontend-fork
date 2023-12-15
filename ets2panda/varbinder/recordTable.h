/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ES2PANDA_VARBINDER_RECORDTABLE_H
#define ES2PANDA_VARBINDER_RECORDTABLE_H

#include "macros.h"
#include "utils/arena_containers.h"
#include "util/ustring.h"
#include "util/enumbitops.h"

namespace panda::es2panda::parser {
class Program;
}  // namespace panda::es2panda::parser

namespace panda::es2panda::checker {
class Signature;
}  // namespace panda::es2panda::checker

namespace panda::es2panda::ir {
class ClassDefinition;
class TSInterfaceDeclaration;
class Identifier;
}  // namespace panda::es2panda::ir

namespace panda::es2panda::varbinder {
class FunctionScope;
class BoundContext;

enum class RecordTableFlags : uint32_t {
    NONE = 0U,
    EXTERNAL = 1U << 0U,
};

DEFINE_BITOPS(RecordTableFlags)

class RecordTable {
public:
    explicit RecordTable(ArenaAllocator *allocator, parser::Program *program, RecordTableFlags flags)
        : classDefinitions_(allocator->Adapter()),
          interfaceDeclarations_(allocator->Adapter()),
          signatures_(allocator->Adapter()),
          program_(program),
          flags_(flags)
    {
    }

    NO_COPY_SEMANTIC(RecordTable);
    NO_MOVE_SEMANTIC(RecordTable);

    ~RecordTable() = default;

    bool IsExternal() const
    {
        return (flags_ & RecordTableFlags::EXTERNAL) != 0;
    }

    ArenaSet<ir::ClassDefinition *> &ClassDefinitions()
    {
        return classDefinitions_;
    }

    const ArenaSet<ir::ClassDefinition *> &ClassDefinitions() const
    {
        return classDefinitions_;
    }

    ArenaSet<ir::TSInterfaceDeclaration *> &InterfaceDeclarations()
    {
        return interfaceDeclarations_;
    }

    const ArenaSet<ir::TSInterfaceDeclaration *> &InterfaceDeclarations() const
    {
        return interfaceDeclarations_;
    }

    ArenaVector<FunctionScope *> &Signatures()
    {
        return signatures_;
    }

    const ArenaVector<FunctionScope *> &Signatures() const
    {
        return signatures_;
    }

    void SetClassDefinition(ir::ClassDefinition *classDefinition)
    {
        record_ = classDefinition;
    }

    ir::ClassDefinition *ClassDefinition()
    {
        return std::holds_alternative<ir::ClassDefinition *>(record_) ? std::get<ir::ClassDefinition *>(record_)
                                                                      : nullptr;
    }

    const ir::ClassDefinition *ClassDefinition() const
    {
        return std::holds_alternative<ir::ClassDefinition *>(record_) ? std::get<ir::ClassDefinition *>(record_)
                                                                      : nullptr;
    }

    void SetInterfaceDeclaration(ir::TSInterfaceDeclaration *interfaceDeclaration)
    {
        record_ = interfaceDeclaration;
    }

    ir::TSInterfaceDeclaration *InterfaceDeclaration()
    {
        return std::holds_alternative<ir::TSInterfaceDeclaration *>(record_)
                   ? std::get<ir::TSInterfaceDeclaration *>(record_)
                   : nullptr;
    }

    const ir::TSInterfaceDeclaration *InterfaceDeclaration() const
    {
        return std::holds_alternative<ir::TSInterfaceDeclaration *>(record_)
                   ? std::get<ir::TSInterfaceDeclaration *>(record_)
                   : nullptr;
    }

    void SetProgram(parser::Program *program)
    {
        program_ = program;
    }

    parser::Program *Program()
    {
        return program_;
    }

    const parser::Program *Program() const
    {
        return program_;
    }

    util::StringView RecordName() const;

private:
    friend class BoundContext;
    using RecordHolder = std::variant<ir::ClassDefinition *, ir::TSInterfaceDeclaration *, std::nullptr_t>;

    ArenaSet<ir::ClassDefinition *> classDefinitions_;
    ArenaSet<ir::TSInterfaceDeclaration *> interfaceDeclarations_;
    ArenaVector<varbinder::FunctionScope *> signatures_;
    RecordHolder record_ {nullptr};
    parser::Program *program_ {};
    BoundContext *boundCtx_ {};
    RecordTableFlags flags_ {};
};

class BoundContext {
public:
    explicit BoundContext(RecordTable *recordTable, ir::ClassDefinition *classDef);
    explicit BoundContext(RecordTable *recordTable, ir::TSInterfaceDeclaration *interfaceDecl);
    ~BoundContext();

    NO_COPY_SEMANTIC(BoundContext);
    NO_MOVE_SEMANTIC(BoundContext);

    void *operator new(size_t) = delete;
    void *operator new[](size_t) = delete;

    util::StringView FormRecordName() const;

private:
    BoundContext *prev_;
    RecordTable *recordTable_;
    RecordTable::RecordHolder savedRecord_ {nullptr};
    ir::Identifier *recordIdent_;
};

}  // namespace panda::es2panda::varbinder

#endif  // ES2PANDA_VARBINDER_RECORDTABLE_H
