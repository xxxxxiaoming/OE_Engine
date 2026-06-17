import json
with open('scene.gltf', encoding='utf-8') as f:
    data = json.load(f)
for i, m in enumerate(data.get('meshes', [])):
    mat_idx = m['primitives'][0].get('material')
    mat_name = data['materials'][mat_idx]['name'] if mat_idx is not None else "None"
    print(f"Mesh {i}: {m.get('name')} -> Material {mat_idx} ({mat_name})")
