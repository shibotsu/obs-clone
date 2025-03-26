import { useState } from "react";
import { useNavigate } from "react-router-dom";
import {
  TextField,
  DefaultButton,
  Stack,
  PrimaryButton,
} from "@fluentui/react";
import "./Auth.css";

const LoginPage = () => {
  const [password, setPassword] = useState("");
  const [usernameOrEmail, setUsernameOrEmail] = useState("");

  let navigate = useNavigate();

  const handleSubmit = () => {
    const isEmail = usernameOrEmail.includes("@");
    const requestData = {
      username: isEmail ? null : usernameOrEmail,
      email: isEmail ? usernameOrEmail : null,
      password: password,
    };
    console.log(
      "Login data: " +
        requestData.username +
        " " +
        requestData.email +
        " " +
        requestData.password
    );
    navigate("/");
  };

  return (
    <div className="login-container">
      <Stack tokens={{ childrenGap: 10 }}>
        <TextField
          label="Username or Email"
          value={usernameOrEmail}
          onChange={(e, newValue) => setUsernameOrEmail(newValue || "")}
          required
        />
        <TextField
          label="Password"
          type="password"
          value={password}
          onChange={(e, newValue) => setPassword(newValue || "")}
          required
        />
        <PrimaryButton
          text="Login"
          onClick={handleSubmit}
          className="custom-primary-button"
        />
      </Stack>
    </div>
  );
};

export default LoginPage;
