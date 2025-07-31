# QMetaModel (QForge)

Declarative Qt data‑model generator (Table & Tree) powered by YAML / JSON DSL.  

Stop writing boilerplate Qt models — describe them once, QForge does the rest.**

---

## Why QForge?

* **No boilerplate** — forget hundreds of lines of `QAbstractItemModel` code.

* **YAML / JSON DSL** — human‑readable model specs stored in your Qt resources.

* **One‑line integration** — pass a `QSqlDatabase` (or your own callback) to the constructor — done.

* **Async when you need it** — just call `execute(..., /*threaded=*/true)` and keep UI 60 fps.

* **Hooks** — intercept every stage (`preExecute`, `postExecute`, `rowMapped`, `errorCaught`).

* **Qt‑native** — pure Qt 6 / C++17, PIMPL, no GUI dependencies beyond Qt itself.

* **GPL v3 + Commercial** — free for GPL projects, affordable licenses for proprietary code.

---

## Quick start

### 1. Install
```bash

# add as submodule

git submodule add https://github.com/MrHoldem/QMetaModel.git deps/qforge

# CMakeLists.txt

add_subdirectory(deps/qforge)

target_link_libraries(myApp PRIVATE QForge)

```
### 2. Register default backend *(optional)*
```cpp

#include <QForge/EngineRegistry>

#include <QSqlDatabase>

int main(int argc, char** argv)

{

    QCoreApplication app(argc, argv);

    // create shared DB connection

    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");

    db.setDatabaseName("app"); db.open();

    // register once — all models will use it if you don't pass anything else

    QForge::EngineRegistry::registerDefaultBackend(db);

    // ...

    return app.exec();

}

```

### 3. Describe your model (`:/models/Projects.yaml`)
```yaml

name: Projects

loadQuery: select_all

columns:

  - { name: id,        type: uuid,     key: true,  visible: false }

  - { name: title,     type: string,   editable: true }

  - { name: goal,      type: string,   editable: true }

  - { name: created_at,type: datetime }

queries:

  select_all:

    sql: "SELECT * FROM ap.target_albums ORDER BY created_at DESC"

  remove:

    args: [ { name: id, type: uuid } ]

    sql: "DELETE FROM ap.target_albums WHERE id = ${id}"

```

### 4. Use in code
```cpp

#include <QForge/TableModel>

// Constructor (dsl-resource, QSqlDatabase, parent)

auto model = new QForge::TableModel(":/models/Projects.yaml",

                                    QSqlDatabase::database(), this);

view->setModel(model);

```

---

## ⚙️ Advanced usage

```cpp

// Custom data source (CSV, REST, etc.)

QForge::DbCallback csvCb = [](const QForge::QueryContext& ctx)

                           -> QForge::QueryResult { /* ... */ };

auto csvModel = new QForge::TableModel(":/models/CsvView.yaml",

                                       csvCb, this);

// Async execution + hook

csvModel->setHook(QForge::Hook::RowMapped,

                  [](QVariantHash& row){

                      row["_color"] = row["goal"].toString().contains("Urgent")

                                       ? QColor(Qt::red) : QColor(Qt::white);

                      return true;

                  });

csvModel->execute("select_all", {}, /*threaded=*/true);

```

---

## Threaded execution

* `execute(queryName, args, true)` runs in `QThreadPool`.

* Result is delivered back via signal **`executeFinished(id, result)`**.

---

## 🔒 License

* **GPL v3** — free for open‑source (GPL‑compatible) software.  

* **Commercial licenses** — start at **€49 / developer**.  

     * Contact **aleksandrfurmanoa@gmail.com** for details.
