// ✅ Backend API URL
const apiBase = 'https://library-management-system-2-th0o.onrender.com';

// ✅ Fetch Books
async function fetchBooks() {
  const res = await fetch(`${apiBase}/books`);
  if (!res.ok) throw new Error('Failed to fetch');
  return await res.json();
}

// ✅ Render Table
function renderCatalog() {
  const tbody = document.querySelector('#booksTable tbody');
  if (!tbody) return;
  tbody.innerHTML = '';
  fetchBooks().then(arr => {
    arr.forEach(b => {
      const tr = document.createElement('tr');
      tr.innerHTML = `
        <td>${b.id}</td>
        <td>${b.title}</td>
        <td>${b.author}</td>
        <td>${b.issued ? 'Issued' : 'Available'}</td>
        <td>
          ${b.issued ? `<button data-id="${b.id}" data-action="return">Return</button>` :
                       `<button data-id="${b.id}" data-action="issue">Issue</button>`}
          <button data-id="${b.id}" data-action="delete">Delete</button>
        </td>`;
      tbody.appendChild(tr);
    });
  }).catch(() => {
    tbody.innerHTML = '<tr><td colspan="5">Error loading books</td></tr>';
  });
}

// ✅ Button Actions
document.addEventListener('click', e => {
  const btn = e.target.closest('button');
  if (!btn) return;

  const id = btn.dataset.id;
  const action = btn.dataset.action;

  if (action === 'issue') {
    fetch(`${apiBase}/issue`, {
      method: 'POST',
      headers: {'Content-Type':'application/json'},
      body: JSON.stringify({ studentId: "STU001", bookId: parseInt(id) })
    }).then(() => renderCatalog());
  }

  if (action === 'return') {
    fetch(`${apiBase}/return`, {
      method: 'POST',
      headers: {'Content-Type':'application/json'},
      body: JSON.stringify({ bookId: parseInt(id) })
    }).then(() => renderCatalog());
  }

  if (action === 'delete') {
    if (!confirm('Delete book id ' + id + '?')) return;
    fetch(`${apiBase}/books/${id}`, { method: 'DELETE' })
      .then(() => renderCatalog());
  }
});

// ✅ On Page Load
document.addEventListener('DOMContentLoaded', () => {
  if (document.querySelector('#booksTable')) renderCatalog();

  // Add Book
  const addForm = document.getElementById('addForm');
  if (addForm) {
    addForm.addEventListener('submit', e => {
      e.preventDefault();
      const id = parseInt(document.getElementById('bookId').value);
      const title = document.getElementById('bookTitle').value;
      const author = document.getElementById('bookAuthor').value;

      fetch(`${apiBase}/books`, {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({ id, title, author })
      }).then(r => {
        if (r.ok) {
          alert('Book Added ✅');
          addForm.reset();
          renderCatalog();
        } else {
          r.json().then(j => alert('Error: ' + (j.error || 'unknown')));
        }
      });
    });
  }

  // Delete Book (Admin form)
  const delForm = document.getElementById('deleteForm');
  if (delForm) {
    delForm.addEventListener('submit', e => {
      e.preventDefault();
      const id = parseInt(document.getElementById('delId').value);

      fetch(`${apiBase}/books/${id}`, { method: 'DELETE' })
        .then(() => {
          alert('Book Deleted ✅');
          delForm.reset();
          renderCatalog();
        });
    });
  }
});
