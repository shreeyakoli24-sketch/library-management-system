const apiBase = '/api';

async function fetchBooks() {
  const res = await fetch(`${apiBase}/books`);
  if (!res.ok) throw new Error('Failed to fetch');
  return await res.json();
}

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
        </td>
      `;
      tbody.appendChild(tr);
    });
  }).catch(e => {
    tbody.innerHTML = '<tr><td colspan="5">Error loading books</td></tr>';
  });
}

document.addEventListener('click', e => {
  const btn = e.target.closest('button');
  if (!btn) return;
  const id = btn.dataset.id;
  const action = btn.dataset.action;
  if (!id || !action) return;

  if (action === 'issue' || action === 'return') {
    fetch(`${apiBase}/books/${id}/${action}`, { method: 'PUT' })
      .then(r => r.json()).then(() => renderCatalog());
  } else if (action === 'delete') {
    if (!confirm('Delete book id ' + id + '?')) return;
    fetch(`${apiBase}/books/${id}`, { method: 'DELETE' })
      .then(r => renderCatalog());
  }
});

// Admin forms
document.addEventListener('DOMContentLoaded', () => {
  // render on catalog page
  if (document.querySelector('#booksTable')) renderCatalog();

  const addForm = document.getElementById('addForm');
  if (addForm) {
    addForm.addEventListener('submit', e => {
      e.preventDefault();
      const id = parseInt(document.getElementById('bookId').value, 10);
      const title = document.getElementById('bookTitle').value;
      const author = document.getElementById('bookAuthor').value;
      fetch(`${apiBase}/books`, {
        method: 'POST',
        headers: {'Content-Type':'application/json'},
        body: JSON.stringify({id, title, author})
      }).then(r => {
        if (r.ok) {
          alert('Added');
          addForm.reset();
          renderCatalog();
        } else {
          r.json().then(j => alert('Error: '+(j.error||'unknown')));
        }
      });
    });
  }

  const delForm = document.getElementById('deleteForm');
  if (delForm) {
    delForm.addEventListener('submit', e => {
      e.preventDefault();
      const id = parseInt(document.getElementById('delId').value, 10);
      fetch(`${apiBase}/books/${id}`, { method: 'DELETE' })
        .then(r => {
          if (r.ok) {
            alert('Deleted');
            delForm.reset();
            renderCatalog();
          } else {
            alert('Delete failed');
          }
        });
    });
  }
});