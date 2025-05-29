# Advanced usage
In [basic usage document](basic_usage.md), you need to manually define your own json file to create your 3D assets. However, this is non user-friendly. Therefore, an advanced usage is to tell LLMs to create the json file for you by just typing a single prompt. Below is the prompt to tell LLMs the format of the json file and after that we can create our own 3D assets by a single prompt!

## LLM CSG JSON format prompt
You are generating a JSON file representing a Constructive Solid Geometry (CSG) scene made of Signed Distance Field (SDF) primitives. Each node represents either a primitive or a Boolean operation. Use the following format:

Each node has:
- "type": an integer from the enum `SDFType`:
  - 0 = SPHERE
  - 1 = BOX
  - 2 = PLANE
  - 3 = TRI_PRISM
  - 10 = UNION
  - 11 = INTERSECT
  - 12 = SUBTRACT

- "left": the index of the left child (use -1 for leaf primitives)
- "right": the index of the right child (use -1 for leaf primitives)
- "param1": a vec4 depending on the node type
  - for primitives: param1 = center or position
- "param2": a vec4 depending on the node type
  - for primitives: param2 = size/radius/etc.
- "color": a vec3 RGB color value for visualization

📌 Rules:
- Each Boolean node must refer to valid earlier node indices in the "left" and "right" fields.
- Leaf nodes (primitives) use -1 for "left" and "right".
- Output a valid JSON file with key `"nodes"` and an array of such node objects.
- Only output the raw JSON.

Example prompt:
“Generate a simple CSG SDF scene using one box and one sphere, subtract the sphere from the box.”

Expected output (abbreviated):
```json
{
  "nodes": [
    { "type": 1, "left": -1, "right": -1, "param1": [...], ... },
    { "type": 0, "left": -1, "right": -1, "param1": [...], ... },
    { "type": 12, "left": 0, "right": 1, "param1": [...], ... }
  ]
}
```

The json schema is

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "properties": {
    "nodes": {
      "type": "array",
      "items": {
        "type": "object",
        "properties": {
          "type": { "type": "integer" },
          "left": { "type": "integer" },
          "right": { "type": "integer" },
          "param1": {
            "type": "array",
            "items": { "type": "number" },
            "minItems": 4,
            "maxItems": 4
          },
          "param2": {
            "type": "array",
            "items": { "type": "number" },
            "minItems": 4,
            "maxItems": 4
          },
          "color": {
            "type": "array",
            "items": { "type": "number" },
            "minItems": 3,
            "maxItems": 3
          }
        },
        "required": ["type", "left", "right", "param1", "param2", "color"]
      }
    }
  },
  "required": ["nodes"]
}
```
## LLM prompt
Prompt: Create a robot
### ChatGPT-4o
### Gemini
(Json)[../SDFBooleanEngine/assets/gemini_robot.json]